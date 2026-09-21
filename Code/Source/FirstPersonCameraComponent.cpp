
#include "FirstPersonCameraComponent.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace ModularCharacterController
{
    AZ_COMPONENT_IMPL(FirstPersonCameraComponent, "FirstPersonCameraComponent", "{2FF67FCB-9CFD-4802-B629-E7A506B6A085}");

    void FirstPersonCameraComponent::Activate()
    {
        FirstPersonCameraRequestBus::Handler::BusConnect(GetEntityId());
        AZ::TickBus::Handler::BusConnect();

        // Broadcast bus - connects without an address. Its connection policy replays the
        // current state immediately, so OnActiveViewChanged fires before this call returns
        // and the ViewMode handler ends up connected or not with no frame of uncertainty.
        Camera::CameraNotificationBus::Handler::BusConnect();

        if (m_cameraEntityId.IsValid())
        {
            // Nothing may be sent to the camera until its entity is active: its Camera and
            // Transform components subscribe to their own buses in their own Activate(), and
            // an Event to an address with no handler is a silent no-op - no error, no log.
            // This subscription turns "ready or not ready" into one callback at the right
            // moment, and if the entity is already active it arrives inside BusConnect.
            AZ::EntityBus::Handler::BusConnect(m_cameraEntityId);
        }
        else
        {
            AZ_Warning("FirstPersonCameraComponent", false,
                "Camera Entity is not set. The view will never activate and pitch will go "
                "nowhere - assign the child camera entity in the component properties.");
        }
    }

    void FirstPersonCameraComponent::Deactivate()
    {
        // Disconnecting a handler that was never connected is a guarded no-op, so the
        // ViewMode line is safe even if this component never became the active view.
        ViewModeRequestBus::Handler::BusDisconnect();
        Camera::CameraNotificationBus::Handler::BusDisconnect();
        AZ::EntityBus::Handler::BusDisconnect();
        AZ::TickBus::Handler::BusDisconnect();
        FirstPersonCameraRequestBus::Handler::BusDisconnect();
    }

    int FirstPersonCameraComponent::GetTickOrder()
    {
        // Later than ViewAngles, which sits at TICK_GAME - 1 and updates the angles read
        // below. MovementComponent also sits at TICK_GAME, but the two never exchange
        // anything, so their relative order does not matter.
        return AZ::ComponentTickBus::TICK_GAME;
    }

    void FirstPersonCameraComponent::OnEntityActivated([[maybe_unused]] const AZ::EntityId& entityId)
    {
        // The camera entity is fully active now, which means its Camera component has
        // subscribed and this call will actually reach someone.
        Camera::CameraRequestBus::Event(
            m_cameraEntityId, &Camera::CameraRequestBus::Events::MakeActiveView);

        // One-shot. An entity does not activate twice without being deactivated first, and
        // this component has nothing to do on that path. Staying subscribed would start to
        // matter only once characters are spawned and despawned at runtime.
        AZ::EntityBus::Handler::BusDisconnect();
    }

    void FirstPersonCameraComponent::OnActiveViewChanged(const AZ::EntityId& cameraEntityId)
    {
        // The active view is a single global slot, and this is how a view component learns
        // it won or lost it. Connecting only while active is what keeps the handler count on
        // ViewModeRequestBus at exactly one - there is no separate "am I active" flag that
        // could fall out of sync with reality.
        if (cameraEntityId == m_cameraEntityId)
        {
            ViewModeRequestBus::Handler::BusConnect(GetEntityId());
        }
        else
        {
            ViewModeRequestBus::Handler::BusDisconnect();
        }
    }

    void FirstPersonCameraComponent::OnTick(
        [[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        // Local, not a member: the value is produced and consumed inside this call and has
        // no reason to survive the tick.
        float pitchDegrees = 0.0f;
        ViewAnglesRequestBus::EventResult(
            pitchDegrees, GetEntityId(), &ViewAnglesRequestBus::Events::GetPitchDegrees);

        // First person reads pitch as "tilt the head in place": the camera turns around its
        // own X axis and the eye stays put. An orbiting third person view would read the very
        // same number as "travel along an arc around the character". That difference is the
        // whole reason applying pitch lives here instead of in ViewAngles.
        //
        // Absolute write, not an increment - ViewAngles owns the number, this is only its
        // picture. Yaw is not touched at all: the camera is a child entity and inherits the
        // body's rotation for free.
        //
        // Rotation only. The local translation of this same entity belongs to whoever ends up
        // owning the eye position - crouch offset, head bob, landing dip - which is why this
        // is SetLocalRotationQuaternion and never SetLocalTM.
        AZ::TransformBus::Event(m_cameraEntityId,
            &AZ::TransformBus::Events::SetLocalRotationQuaternion,
            AZ::Quaternion::CreateRotationX(AZ::DegToRad(pitchDegrees)));
    }

    void FirstPersonCameraComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<FirstPersonCameraComponent, AZ::Component>()
                ->Version(1)
                ->Field("CameraEntity", &FirstPersonCameraComponent::m_cameraEntityId)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<FirstPersonCameraComponent>("First Person Camera",
                    "Drives a camera from the character's look direction, seen from the eyes. "
                    "Makes its camera the active view, tilts it to match the pitch owned by "
                    "View Angles, and tells View Angles that the body turns with the look. "
                    "The camera must be a separate child entity carrying a Camera component; "
                    "this one only rotates it, never moves it. Requires View Angles on the "
                    "same entity.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Modular Character Controller/View")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))

                    ->DataElement(AZ::Edit::UIHandlers::EntityId, &FirstPersonCameraComponent::m_cameraEntityId,
                        "Camera Entity",
                        "The child entity holding the Camera component. Leave it empty and this "
                        "component does nothing at all - it has no camera to drive.")
                    ;
            }
        }

        if (AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<FirstPersonCameraComponent>("FirstPersonCamera Component Group")
                ->Attribute(AZ::Script::Attributes::Category, "ModularCharacterController Gem Group")
                ;
        }
    }

    void FirstPersonCameraComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("FirstPersonCameraComponentService"));
    }

    void FirstPersonCameraComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        // Only against a second copy of itself. A third person view component is meant to
        // live on the same entity alongside this one - the active view slot is what decides
        // which of them is driving, not the component list.
        incompatible.push_back(AZ_CRC_CE("FirstPersonCameraComponentService"));
    }

    void FirstPersonCameraComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        // Required services are checked on THIS entity. That is why the camera is not listed
        // here: it lives on a child entity, and demanding it here would simply prevent this
        // component from ever activating.
        required.push_back(AZ_CRC_CE("ViewAnglesComponentService"));
    }

    void FirstPersonCameraComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }
} // namespace ModularCharacterController
