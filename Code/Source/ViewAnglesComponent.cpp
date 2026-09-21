
#include "ViewAnglesComponent.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

#include <ModularCharacterController/ViewModeInterface.h>

#include <AzCore/Component/TransformBus.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/Math/MathUtils.h>   // AZ::DegToRad
#include <AzCore/std/algorithm.h>    // AZStd::clamp

namespace ModularCharacterController
{
    AZ_COMPONENT_IMPL(ViewAnglesComponent, "ViewAnglesComponent", "{586534C0-8C71-4F9A-A8CF-7EF38C0C3E27}");

    void ViewAnglesComponent::Activate()
    {
        ViewAnglesRequestBus::Handler::BusConnect(GetEntityId());
        AZ::TickBus::Handler::BusConnect();

        StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(LookMouseXEventId);
        StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(LookMouseYEventId);
        StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(LookStickXEventId);
        StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(LookStickYEventId);
    }

    void ViewAnglesComponent::Deactivate()
    {
        StartingPointInput::InputEventNotificationBus::MultiHandler::BusDisconnect();
        AZ::TickBus::Handler::BusDisconnect();
        ViewAnglesRequestBus::Handler::BusDisconnect();
    }

    void ViewAnglesComponent::OnTick(float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        // Mouse deltas are already angular increments: scaling them by deltaTime would tie
        // the sensitivity to the frame rate. Stick values are rates, so for them it is
        // mandatory - the same distinction that decided the two sensitivity units.
        const float mouseYaw = -m_fMouseDeltaX * m_fMouseSensitivityX;
        const float stickYaw = -m_fStickX * m_fStickSensitivityX * deltaTime;

        const float mousePitch = -m_fMouseDeltaY * m_fMouseSensitivityY * (m_bInvertMouseY ? -1.0f : 1.0f);
        const float stickPitch = -m_fStickY * m_fStickSensitivityY * deltaTime * (m_bInvertStickY ? -1.0f : 1.0f);

        m_fYawDegrees += mouseYaw + stickYaw;

        // Pitch is clamped rather than wrapped: the head does not roll over backwards.
        // This clamp is the reason pitch has to be stored as a number at all.
        m_fPitchDegrees = AZStd::clamp(
            m_fPitchDegrees + mousePitch + stickPitch, -m_fMaxPitchDegrees, m_fMaxPitchDegrees);

        // Yaw wraps instead. Letting it grow forever costs float precision - past a few
        // hundred thousand degrees the smallest representable step stops being smaller
        // than a mouse twitch, and the view starts snapping. In practice these loops run
        // zero or one time per tick.
        while (m_fYawDegrees > 180.0f) { m_fYawDegrees -= 360.0f; }
        while (m_fYawDegrees < -180.0f) { m_fYawDegrees += 360.0f; }

        // The accumulated mouse motion has now been applied exactly once. This line is the
        // defined reset point the accumulator needs; without it every frame would re-apply
        // every delta ever received.
        m_fMouseDeltaX = 0.0f;
        m_fMouseDeltaY = 0.0f;

        if (ShouldBodyFollowYaw())
        {
            ApplyBodyYaw();
        }
    }

    int ViewAnglesComponent::GetTickOrder()
    {
        // One step ahead of TICK_GAME, where MovementComponent reads the body rotation
        // this component writes. Handlers sharing a tick order run in an undefined order,
        // so being first has to be stated rather than assumed - otherwise movement would
        // sometimes follow last frame's heading.
        return AZ::ComponentTickBus::TICK_GAME - 1;
    }

    void ViewAnglesComponent::OnPressed(float value)
    {
        // A movement that starts from rest arrives as OnPressed and a continuing one as
        // OnHeld - exactly one of the three fires per input event. Handling only one of
        // them would drop either the first twitch or all sustained motion.
        OnHeld(value);
    }

    void ViewAnglesComponent::OnHeld(float value)
    {
        const auto* currentId = StartingPointInput::InputEventNotificationBus::GetCurrentBusId();
        if (!currentId) { return; }

        // Mouse events each carry a piece of motion. Overwriting would silently throw away
        // everything that arrived earlier in the same frame, so they accumulate.
        if (*currentId == LookMouseXEventId) { m_fMouseDeltaX += value; }
        else if (*currentId == LookMouseYEventId) { m_fMouseDeltaY += value; }
        // Stick events carry a position, not a movement. Accumulating a held stick would
        // grow the value without bound within a second.
        else if (*currentId == LookStickXEventId) { m_fStickX = value; }
        else if (*currentId == LookStickYEventId) { m_fStickY = value; }
    }

    void ViewAnglesComponent::OnReleased([[maybe_unused]] float value)
    {
        const auto* currentId = StartingPointInput::InputEventNotificationBus::GetCurrentBusId();
        if (!currentId) { return; }

        // Only the stick needs clearing, and it needs it badly: its latched value is what
        // keeps the view turning, so without this the camera spins forever once the player
        // lets go. Mouse deltas must NOT be cleared here - whatever accumulated since the
        // last tick is real motion that still has to be applied.
        if (*currentId == LookStickXEventId) { m_fStickX = 0.0f; }
        else if (*currentId == LookStickYEventId) { m_fStickY = 0.0f; }
    }

    bool ViewAnglesComponent::ShouldBodyFollowYaw() const
    {
        // The rule belongs to the active view component: a first person view always turns the
        // body with the look, an orbiting third person view leaves it standing. This component
        // deliberately does not know which mode is running - that is why it asks instead of
        // deciding, and why adding a third view mode will not touch this file.
        //
        // The initial value is the answer for "no view component is active". EventResult leaves
        // the variable untouched when nothing is connected to the address, so this default is not
        // a fallback for an error case - it is the behaviour on an entity that has ViewAngles and
        // no view component at all, which has to stay playable on its own.
        bool followsYaw = true;

        ViewModeRequestBus::EventResult(
            followsYaw, GetEntityId(), &ViewModeRequestBus::Events::GetBodyFollowsYaw);

        return followsYaw;
    }

    void ViewAnglesComponent::ApplyBodyYaw() const
    {
        // Absolute write, not an increment. m_fYawDegrees is the master value and the body
        // rotation is derived from it; RotateAroundLocalZ would make the transform the
        // master instead, and the two would drift apart the moment anything else rotates
        // the body - a teleport, a cutscene, or a view mode that stops following yaw.
        //
        // SetLocalRotation takes radians and overwrites all three euler axes. That is what
        // is wanted here: the body stays upright and pitch belongs to the camera, not to it.
        AZ::TransformBus::Event(GetEntityId(), &AZ::TransformBus::Events::SetLocalRotation,
            AZ::Vector3(0.0f, 0.0f, AZ::DegToRad(m_fYawDegrees)));
    }

    void ViewAnglesComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<ViewAnglesComponent, AZ::Component>()
                ->Version(1)
                ->Field("MouseSensitivityX", &ViewAnglesComponent::m_fMouseSensitivityX)
                ->Field("MouseSensitivityY", &ViewAnglesComponent::m_fMouseSensitivityY)
                ->Field("StickSensitivityX", &ViewAnglesComponent::m_fStickSensitivityX)
                ->Field("StickSensitivityY", &ViewAnglesComponent::m_fStickSensitivityY)
                ->Field("InvertMouseY", &ViewAnglesComponent::m_bInvertMouseY)
                ->Field("InvertStickY", &ViewAnglesComponent::m_bInvertStickY)
                ->Field("MaxPitchDegrees", &ViewAnglesComponent::m_fMaxPitchDegrees)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<ViewAnglesComponent>("View Angles",
                    "Owns the character's look direction. Reads mouse and gamepad look input, keeps "
                    "yaw and pitch, clamps pitch, and rotates the body to match the yaw. It does not "
                    "touch any camera: a view component reads the pitch and applies it to its own "
                    "camera, which is what lets first and third person views coexist. Requires an "
                    "Input component with look bindings on the same entity.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Modular Character Controller/View")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))

                    ->DataElement(AZ::Edit::UIHandlers::Default, &ViewAnglesComponent::m_fMouseSensitivityX,
                        "Mouse Sensitivity X",
                        "Degrees of yaw per unit of raw mouse delta. Frame rate independent by nature - "
                        "a mouse reports how far it moved, not how fast.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &ViewAnglesComponent::m_fMouseSensitivityY,
                        "Mouse Sensitivity Y",
                        "Degrees of pitch per unit of raw mouse delta.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &ViewAnglesComponent::m_fStickSensitivityX,
                        "Stick Sensitivity X",
                        "Degrees of yaw per second at full stick deflection. A different unit from the "
                        "mouse values above, which is why the numbers are so much larger.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &ViewAnglesComponent::m_fStickSensitivityY,
                        "Stick Sensitivity Y",
                        "Degrees of pitch per second at full stick deflection.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &ViewAnglesComponent::m_bInvertMouseY,
                        "Invert Mouse Y", "Pushing the mouse forward looks down instead of up.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &ViewAnglesComponent::m_bInvertStickY,
                        "Invert Stick Y", "Pushing the stick forward looks down instead of up.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &ViewAnglesComponent::m_fMaxPitchDegrees,
                        "Max Pitch",
                        "How far up and down the view can look, in degrees, applied symmetrically. "
                        "Keep it below 90: at exactly 90 the horizontal part of the look direction "
                        "vanishes, and anything deriving a heading from it breaks.")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.0f)
                    ->Attribute(AZ::Edit::Attributes::Max, 89.9f)
                    ;
            }
        }

        if (AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<ViewAnglesComponent>("ViewAngles Component Group")
                ->Attribute(AZ::Script::Attributes::Category, "ModularCharacterController Gem Group")
                ;
        }
    }

    void ViewAnglesComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("ViewAnglesComponentService"));
    }

    void ViewAnglesComponent::GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        // Pushing our own service prevents a second copy on the same entity: two owners of
        // one look direction would fight over the body rotation every frame.
        incompatible.push_back(AZ_CRC_CE("ViewAnglesComponentService"));
    }

    void ViewAnglesComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("InputConfigurationService"));
    }

    void ViewAnglesComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }
} // namespace ModularCharacterController
