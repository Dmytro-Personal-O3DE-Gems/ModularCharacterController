
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

    //! Below this weight the bob is under a tenth of a millimetre, which is where the
    //! cycle may be restarted without the restart being visible.
    static constexpr float BobWeightEpsilon = 0.001f;

    void ViewOffsetComponent::Activate()
    {
        ViewOffsetRequestBus::Handler::BusConnect(GetEntityId());
        GroundTrackerNotificationBus::Handler::BusConnect(GetEntityId());
        AZ::TickBus::Handler::BusConnect();
    }

    void ViewOffsetComponent::Deactivate()
    {
        ViewOffsetRequestBus::Handler::BusDisconnect(GetEntityId());
        GroundTrackerNotificationBus::Handler::BusDisconnect(GetEntityId());
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
        m_vTotalOffset = AZ::Vector3(0.0f, 0.0f, m_fCrouchOffsetZ + m_fLandingDipZ) + m_vBobOffset;
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

        // Everything that used to switch the effect off now feeds one continuous number.
        // A boolean gate on a continuous output steps at the instant it flips, and nothing
        // done to whatever produces the boolean can smooth that step away: a jump used to
        // cut the bob mid-cycle, moving the eye a centimetre and a half in a single frame.
        // Speed belongs in the same number rather than beside it - walking slowly is less
        // bob for the same reason being airborne is none.
        const bool isWalking = isGrounded && horizontalSpeed >= MinBobSpeed;
        const float targetWeight = (m_bEnableBob && isWalking)
            ? AZStd::min(horizontalSpeed / m_fBobReferenceSpeed, 1.0f)
            : 0.0f;

        m_fBobWeight +=
            (targetWeight - m_fBobWeight) * AZStd::clamp(m_fBobBlendRate * deltaTime, 0.0f, 1.0f);

        if (isWalking)
        {
            // Phase advances with distance covered, not with time: one full gait cycle per
            // stride length, whatever the frame rate. Wrapped so the accumulator cannot grow
            // until float precision starts to show - the same reason ViewAngles wraps yaw.
            m_fBobPhase += horizontalSpeed * deltaTime * (AZ::Constants::TwoPi / m_fBobStrideLength);
            while (m_fBobPhase > AZ::Constants::TwoPi)
            {
                m_fBobPhase -= AZ::Constants::TwoPi;
            }
        }
        else if (m_fBobWeight < BobWeightEpsilon)
        {
            // The cycle freezes the moment the feet stop working and is restarted only once
            // the weight has faded to nothing, where the amplitude is already zero and moving
            // the phase cannot show. Restarting at zero is what makes the first step after a
            // landing read as a step rather than as a continuation of the old gait.
            m_fBobWeight = 0.0f;
            m_fBobPhase = 0.0f;
        }

        // Figure of eight. The body rises on every step - twice per gait cycle - while it
        // sways left and right once per cycle, so the vertical axis runs at double frequency.
        // Both axes read the same phase; two independent counters would drift apart.
        const float side = AZ::Sin(m_fBobPhase) * m_fBobAmplitudeSide * m_fBobWeight;
        const float up = AZ::Sin(m_fBobPhase * 2.0f) * m_fBobAmplitudeUp * m_fBobWeight;

        m_vBobOffset = AZ::Vector3(side, 0.0f, up);
    }

    void ViewOffsetComponent::OnLanded(const LandingInfo& landing)
    {
        // Arming only. This runs synchronously inside GroundTracker's tick, so anything
        // computed here would advance this component's effects at a moment unrelated to
        // its own OnTick. The threshold is left out on purpose too - it is policy, and it
        // belongs next to the knobs that express it.
        m_fPendingImpactSpeed = landing.m_impactSpeed;
    }

    void ViewOffsetComponent::UpdateLandingDip(float deltaTime)
    {
        // Read and cleared in the same breath: a landing can never be applied twice, and
        // a frame without one costs a single float compare.
        const float impactSpeed = m_fPendingImpactSpeed;
        m_fPendingImpactSpeed = 0.0f;

        if (m_bEnableLandingDip && impactSpeed > m_fLandingDipMinSpeed)
        {
            // GroundTracker reports every landing, stepping off a kerb included. Which of
            // them deserve a camera reaction is decided here, not there - footstep sound
            // wants all of them and fall damage wants far fewer, from the same signal.
            const float range = AZStd::max(m_fLandingDipMaxSpeed - m_fLandingDipMinSpeed, 0.001f);
            const float strength = AZStd::clamp((impactSpeed - m_fLandingDipMinSpeed) / range, 0.0f, 1.0f);

            // Subtracted rather than assigned: landing again while still compressed goes
            // deeper, the way a body does. The clamp stops that stacking without limit.
            m_fLandingDipZ = AZStd::max(
                m_fLandingDipZ - strength * m_fLandingDipMaxDepth, -m_fLandingDipMaxDepth);
        }

        // The impact itself is instant - that is what an impact is - and only the recovery
        // is smoothed. Same relaxation as the crouch, so the same rate reads the same way
        // in both places, and the target is spelled out rather than folded into the maths
        // to keep it obvious that this effect always decays back to neutral.
        m_fLandingDipZ += (0.0f - m_fLandingDipZ) * AZStd::clamp(m_fLandingDipRecoveryRate * deltaTime, 0.0f, 1.0f);
    }

    void ViewOffsetComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<ViewOffsetComponent, AZ::Component>()
                ->Version(1)
                ->Field("CrouchSmoothingRate", &ViewOffsetComponent::m_fCrouchSmoothingRate)
                ->Field("EnableBob", &ViewOffsetComponent::m_bEnableBob)
                ->Field("BobBlendRate", &ViewOffsetComponent::m_fBobBlendRate)
                ->Field("BobStrideLength", &ViewOffsetComponent::m_fBobStrideLength)
                ->Field("BobReferenceSpeed", &ViewOffsetComponent::m_fBobReferenceSpeed)
                ->Field("BobAmplitudeUp", &ViewOffsetComponent::m_fBobAmplitudeUp)
                ->Field("BobAmplitudeSide", &ViewOffsetComponent::m_fBobAmplitudeSide)
                ->Field("EnableLandingDip", &ViewOffsetComponent::m_bEnableLandingDip)
                ->Field("LandingDipMinSpeed", &ViewOffsetComponent::m_fLandingDipMinSpeed)
                ->Field("LandingDipMaxSpeed", &ViewOffsetComponent::m_fLandingDipMaxSpeed)
                ->Field("LandingDipMaxDepth", &ViewOffsetComponent::m_fLandingDipMaxDepth)
                ->Field("LandingDipRecoveryRate", &ViewOffsetComponent::m_fLandingDipRecoveryRate)
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

                    ->DataElement(AZ::Edit::UIHandlers::Default, &ViewOffsetComponent::m_fBobBlendRate,
                        "Blend Rate",
                        "How fast the effect fades in and out when the character starts walking, stops, "
                        "jumps or lands. Same kind of rate as the smoothing above, so 8 blends in about "
                        "an eighth of a second. This exists because every condition that silences the "
                        "bob - leaving the ground above all - flips in a single frame, and cutting a "
                        "running cycle mid-stride moves the eye by the full amplitude at once, which "
                        "reads as a glitch. Raise it past 30 and the cut comes back.")
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

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Landing Dip")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &ViewOffsetComponent::m_bEnableLandingDip,
                        "Enable Landing Dip",
                        "Drops the eye on impact and lets it rise back. This is what sells the weight "
                        "of a fall - without it a jump reads as the floor moving rather than the body "
                        "arriving. Like the bob above, it is camera motion the player did not ask for, "
                        "so it gets a switch.")

                    ->DataElement(AZ::Edit::UIHandlers::Default, &ViewOffsetComponent::m_fLandingDipMinSpeed,
                        "Min Impact Speed",
                        "Impacts below this vertical speed, in metres per second, are ignored entirely. "
                        "Ground tracking reports every landing, and most of them are not events: walking "
                        "down a staircase lands the character on every step at roughly 1 to 2 m/s. Around "
                        "2.5 separates those from a deliberate jump. Set it to 0 and the camera twitches "
                        "its way down every flight of stairs.")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.0f)

                    ->DataElement(AZ::Edit::UIHandlers::Default, &ViewOffsetComponent::m_fLandingDipMaxSpeed,
                        "Max Impact Speed",
                        "Speed at which the dip reaches its full depth, in metres per second. Between this "
                        "and the minimum the depth scales linearly, so a hop and a long fall do not look "
                        "the same; above it the effect is clamped, which keeps a fall from a great height "
                        "from burying the camera in the floor. A jump from a standing start lands at "
                        "roughly the jump speed, so set this a good way above that.")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.1f)

                    ->DataElement(AZ::Edit::UIHandlers::Default, &ViewOffsetComponent::m_fLandingDipMaxDepth,
                        "Max Depth",
                        "How far the eye drops at full strength, in metres, and also the hard cap on the "
                        "total when impacts land on top of one another. Five to ten centimetres reads as "
                        "weight; past that the floor appears to swallow the character.")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.0f)

                    ->DataElement(AZ::Edit::UIHandlers::Default, &ViewOffsetComponent::m_fLandingDipRecoveryRate,
                        "Recovery Rate",
                        "How quickly the eye rises back to neutral. Same kind of rate as the crouch "
                        "smoothing above - about 63 percent of the remaining distance every 1/rate "
                        "seconds - so 8 recovers in something under half a second. The drop itself is "
                        "always instant whatever this is set to: only the return is smoothed, because an "
                        "impact that eased in would not read as an impact.")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.1f)
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
