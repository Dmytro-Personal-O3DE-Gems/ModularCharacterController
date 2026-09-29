
#include "ViewOffsetComponent.h"

#include <AzCore/std/algorithm.h>
#include <AzCore/Math/MathUtils.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace ModularCharacterController
{
    AZ_COMPONENT_IMPL(ViewOffsetComponent, "ViewOffsetComponent", "{7834680F-7193-4AAF-B5FC-60FC8271E230}");

    //! Below this speed the bob amplitude is under a millimetre, so the cycle is cut
    //! rather than left running on noise from the physics solver.
    static constexpr float MinBobSpeed = 0.05f;

    void ViewOffsetComponent::Activate()
    {
        ViewOffsetRequestBus::Handler::BusConnect(GetEntityId());
        AZ::TickBus::Handler::BusConnect();
    }

    void ViewOffsetComponent::Deactivate()
    {
        ViewOffsetRequestBus::Handler::BusDisconnect(GetEntityId());
        AZ::TickBus::Handler::BusDisconnect();
    }

    void ViewOffsetComponent::OnTick(float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        // Each effect advances its own state and writes only its own field, so the order
        // between these three does not matter. If it ever starts to matter, one of them is
        // writing into somebody else's field.
        UpdateCrouchOffset(deltaTime);
        UpdateBobOffset(deltaTime);
        UpdateLandingDip(deltaTime);

        // The only place the total is assembled.
        m_vTotalOffset = AZ::Vector3(0.0f, 0.0f, m_fCrouchOffsetZ) + m_vBobOffset;
    }

    void ViewOffsetComponent::SetOffset([[maybe_unused]] AZ::Crc32 channel, [[maybe_unused]] const AZ::Vector3& offset)
    {
    }

    void ViewOffsetComponent::UpdateCrouchOffset(float deltaTime)
    {
        float standingHeight = 0.0f;
        float currentHeight = 0.0f;

        MovementRequestBus::EventResult(standingHeight, GetEntityId(), &MovementRequests::GetStandingCapsuleHeight);
        MovementRequestBus::EventResult(currentHeight, GetEntityId(), &MovementRequests::GetCapsuleHeight);

        // The eye keeps a constant distance from the top of the capsule, and Resize()
        // holds the base in place - so the drop equals the height lost, whoever caused it.
        // Both heights read 0 until OnCharacterActivated fills them in Movement, and 0 - 0
        // is a harmless zero offset.
        const float deltaZ = currentHeight - standingHeight;

        m_fCrouchOffsetZ += (deltaZ - m_fCrouchOffsetZ) * AZStd::clamp(m_fCrouchSmoothingRate * deltaTime, 0.0f, 1.0f);
    }

    void ViewOffsetComponent::UpdateBobOffset(float deltaTime)
    {
        bool isGrounded = false;
        GroundTrackerRequestBus::EventResult(
            isGrounded, GetEntityId(), &GroundTrackerRequests::GetIsGrounded);

        AZ::Vector3 velocity = AZ::Vector3::CreateZero();
        Physics::CharacterRequestBus::EventResult(
            velocity, GetEntityId(), &Physics::CharacterRequests::GetVelocity);

        // Only the horizontal part drives the gait - falling is not walking. Just the
        // magnitude is needed, so it does not matter that velocity arrives in world space
        // while this offset is expressed in the player's local space.
        const float horizontalSpeed =
            AZ::Vector3(velocity.GetX(), velocity.GetY(), 0.0f).GetLength();

        // Airborne, disabled or standing still: no footfalls, so the contribution is zero.
        // Written rather than skipped - a skipped field keeps its last value and would leave
        // the eye frozen off-centre wherever the cycle happened to stop.
        if (!m_bEnableBob || !isGrounded || horizontalSpeed < MinBobSpeed)
        {
            m_vBobOffset = AZ::Vector3::CreateZero();

            // Safe to reset here and nowhere else: the amplitude is already zero at this
            // point, so restarting the cycle cannot show up as a jump.
            m_fBobPhase = 0.0f;
            return;
        }

        // Amplitude follows the actual speed, which is what makes the effect fade out with
        // the body's own deceleration and grow on a sprint, with no special cases for either.
        const float speedFactor = AZStd::min(horizontalSpeed / m_fBobReferenceSpeed, 1.0f);

        // Phase advances with distance covered, not with time: one full gait cycle per stride
        // length, whatever the frame rate. Wrapped so the accumulator cannot grow until float
        // precision starts to show - the same reason ViewAngles wraps yaw.
        m_fBobPhase += horizontalSpeed * deltaTime * (AZ::Constants::TwoPi / m_fBobStrideLength);
        while (m_fBobPhase > AZ::Constants::TwoPi)
        {
            m_fBobPhase -= AZ::Constants::TwoPi;
        }

        // Figure of eight. The body rises on every step - twice per gait cycle - while it
        // sways left and right once per cycle, so the vertical axis runs at double frequency.
        // Both axes read the same phase; two independent counters would drift apart.
        const float side = AZ::Sin(m_fBobPhase) * m_fBobAmplitudeSide * speedFactor;
        const float up = AZ::Sin(m_fBobPhase * 2.0f) * m_fBobAmplitudeUp * speedFactor;

        m_vBobOffset = AZ::Vector3(side, 0.0f, up);
    }

    void ViewOffsetComponent::UpdateLandingDip([[maybe_unused]] float deltaTime)
    {
    }

    void ViewOffsetComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<ViewOffsetComponent, AZ::Component>()
                ->Version(1)
                ->Field("CrouchSmoothingRate", &ViewOffsetComponent::m_fCrouchSmoothingRate)
                ->Field("EnableBob", &ViewOffsetComponent::m_bEnableBob)
                ->Field("BobStrideLength", &ViewOffsetComponent::m_fBobStrideLength)
                ->Field("BobReferenceSpeed", &ViewOffsetComponent::m_fBobReferenceSpeed)
                ->Field("BobAmplitudeUp", &ViewOffsetComponent::m_fBobAmplitudeUp)
                ->Field("BobAmplitudeSide", &ViewOffsetComponent::m_fBobAmplitudeSide)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<ViewOffsetComponent>("View Offset",
                    "Owns where the eye sits relative to its authored position. Reports a pure "
                    "offset and never touches the camera itself - the view component adds it to "
                    "the base, which is what keeps first and third person sharing one answer. "
                    "Built-in effects are computed here, each with its own smoothing; outside "
                    "systems such as weapon recoil or an explosion contribute through SetOffset "
                    "on named channels, and everything is summed once per tick.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Modular Character Controller/View")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Crouch")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &ViewOffsetComponent::m_fCrouchSmoothingRate,
                        "Crouch Smoothing Rate",
                        "How quickly the eye follows the capsule when its height changes. The value "
                        "is a rate, not a duration: the eye covers about 63 percent of the remaining "
                        "distance every 1/rate seconds, so 10 settles in roughly a third of a second "
                        "and 20 reads as almost instant. Below 5 the character feels like it is "
                        "moving through water. The capsule itself resizes in one step - only the view "
                        "is smoothed, so the collision shape never lags behind what the player can "
                        "walk under.")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.0f)

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Head Bob")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &ViewOffsetComponent::m_bEnableBob,
                        "Enable Head Bob",
                        "Procedural camera motion while walking. Some players get motion sick from it, "
                        "which is why this is a switch and the amplitudes below go down to zero - treat "
                        "it as an accessibility setting, not only as a taste one.")

                    ->DataElement(AZ::Edit::UIHandlers::Default, &ViewOffsetComponent::m_fBobStrideLength,
                        "Stride Length",
                        "Metres covered by one full gait cycle - left step plus right step. Around 1.6 "
                        "for a human. The cycle is driven by distance rather than time, so this stays "
                        "correct at any speed: walking and sprinting cover a stride in different "
                        "amounts of time but in the same amount of ground.")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.1f)

                    ->DataElement(AZ::Edit::UIHandlers::Default, &ViewOffsetComponent::m_fBobReferenceSpeed,
                        "Reference Speed",
                        "Speed at which the amplitudes below are reached in full, in metres per second. "
                        "Set it to the walking speed. Slower movement scales the effect down and stops "
                        "it fading out exactly as the body decelerates; faster movement is clamped, so "
                        "a sprint does not double the shake.")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.1f)

                    ->DataElement(AZ::Edit::UIHandlers::Default, &ViewOffsetComponent::m_fBobAmplitudeUp,
                        "Amplitude Up",
                        "How far the eye travels vertically, in metres. Runs at twice the frequency of "
                        "the sideways axis, because the body rises on every step. Two to four "
                        "centimetres reads as walking; past that it turns into a caricature and starts "
                        "making people ill.")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.0f)

                    ->DataElement(AZ::Edit::UIHandlers::Default, &ViewOffsetComponent::m_fBobAmplitudeSide,
                        "Amplitude Side",
                        "How far the eye travels sideways, in metres, once per gait cycle. Usually kept "
                        "smaller than the vertical one: sideways motion moves the crosshair off target "
                        "and is noticed sooner.")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.0f)
                    ;
            }
        }

        if (AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<ViewOffsetComponent>("ViewOffset")
                ->Attribute(AZ::Script::Attributes::Category, "ModularCharacterController Gem Group")
                ;
        }
    }

    void ViewOffsetComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("ViewOffsetComponentService"));
    }

    void ViewOffsetComponent::GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
		incompatible.push_back(AZ_CRC_CE("ViewOffsetComponentService"));
    }

    void ViewOffsetComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
		required.push_back(AZ_CRC_CE("MovementComponentService"));
        required.push_back(AZ_CRC_CE("GroundTrackerComponentService"));
    }

    void ViewOffsetComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }
} // namespace ModularCharacterController
