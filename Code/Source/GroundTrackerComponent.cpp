
#include "GroundTrackerComponent.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

#include <AzCore/Math/Vector3.h>

namespace ModularCharacterController
{
    AZ_COMPONENT_IMPL(GroundTrackerComponent, "GroundTrackerComponent", "{BC307F2C-AA3B-4C69-BF07-DA59FB92EAEA}");

    void GroundTrackerComponent::Activate()
    {
        GroundTrackerRequestBus::Handler::BusConnect(GetEntityId());
        AZ::TickBus::Handler::BusConnect();
    }

    void GroundTrackerComponent::Deactivate()
    {
        GroundTrackerRequestBus::Handler::BusDisconnect(GetEntityId());
        AZ::TickBus::Handler::BusDisconnect();
    }

    void GroundTrackerComponent::OnTick(float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        // Default true on purpose: PhysX's IsOnGround() also returns true when there is no
        // character controller, and that keeps a fresh spawn from looking like a landing.
        bool rawOnGround = true;
        PhysX::CharacterGameplayRequestBus::EventResult(
            rawOnGround, GetEntityId(), &PhysX::CharacterGameplayRequests::IsOnGround);

        if (rawOnGround)
        {
            // Compared against the FILTERED state, not the raw one. A dip below the grace time
            // never flipped m_isGrounded, so stepping off a kerb produces no landing at all -
            // which is the intended behaviour, not a missed event.
            const bool isLanding = !m_isGrounded;

            // Touching ground is trusted immediately. A landing must not be delayed: the sharp
            // onset is what reads as an impact, and postponing it destroys the effect.
            m_isGrounded = true;

            if (isLanding)
            {
                // Reported before the two fields below are cleared. Both values describe the
                // flight that has just ended and cease to exist two lines from here.
                LandingInfo landing;
                landing.m_impactSpeed = m_fLastAirborneImpactSpeed;
                landing.m_airborneTime = m_fTimeSinceGroundLost;

                AZ_Printf("GroundTracker", "Landed: impact %.2f m/s after %.2f s airborne",
                    landing.m_impactSpeed, landing.m_airborneTime);
                GroundTrackerNotificationBus::Event(
                    GetEntityId(), &GroundTrackerNotifications::OnLanded, landing);
            }

            m_fTimeSinceGroundLost = 0.0f;
            m_fLastAirborneImpactSpeed = 0.0f;
            return;
        }

        // Losing ground is not trusted until it has held for the grace time. The engine tests
        // ground with a 2 cm overlap box, so stairs, seams and slope crests produce single
        // frames of "airborne" while the character is in fact walking normally.
        m_fTimeSinceGroundLost += deltaTime;

        if (m_fTimeSinceGroundLost >= m_fGroundLossGraceTime)
        {
            m_isGrounded = false;
        }

        // Captured on every airborne frame because the value is gone by the landing frame:
        // the engine's ApplyGravity() does m_fallingVelocity = Zero() as soon as it sees
        // ground. Whoever asks after landing gets zero, so it has to be remembered in advance.
        AZ::Vector3 fallingVelocity = AZ::Vector3::CreateZero();
        PhysX::CharacterGameplayRequestBus::EventResult(
            fallingVelocity, GetEntityId(), &PhysX::CharacterGameplayRequests::GetFallingVelocity);

        // GetLength(), not GetZ(): the vector points along gravity by construction, so its
        // length is the magnitude - non-negative, and correct under any gravity direction.
        m_fLastAirborneImpactSpeed = fallingVelocity.GetLength();
    }

    void GroundTrackerComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<GroundTrackerComponent, AZ::Component>()
                ->Version(1)
                ->Field("GroundLossGraceTime", &GroundTrackerComponent::m_fGroundLossGraceTime)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<GroundTrackerComponent>("GroundTrackerComponent", "[Description of functionality provided by this component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "ComponentCategory")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ;
            }
        }

        if (AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<GroundTrackerComponent>("GroundTracker Component Group")
                ->Attribute(AZ::Script::Attributes::Category, "ModularCharacterController Gem Group")
                ;
        }
    }

    void GroundTrackerComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("GroundTrackerComponentService"));
    }

    void GroundTrackerComponent::GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
    }

    void GroundTrackerComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("PhysicsCharacterGameplayService"));
    }

    void GroundTrackerComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }
} // namespace ModularCharacterController
