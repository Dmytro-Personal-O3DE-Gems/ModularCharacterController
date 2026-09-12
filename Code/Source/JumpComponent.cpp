
#include "JumpComponent.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace ModularCharacterController
{
    AZ_COMPONENT_IMPL(JumpComponent, "JumpComponent", "{B3252609-1EC1-4961-90C5-81172DDEDB4B}");

    
    void JumpComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<JumpComponent, AZ::Component>()
                ->Version(1)
                ->Field("JumpSpeed", &JumpComponent::m_fJumpSpeed)
                ->Field("CrouchJumpMode", &JumpComponent::m_eCrouchJumpMode)
                ->Field("JumpBufferTime", &JumpComponent::m_fJumpBufferTime)
                ->Field("HeadHitCheckMargin", &JumpComponent::m_fHeadHitMinStallTime)
                ->Field("HeadHitCheckMargin", &JumpComponent::m_fHeadHitStallRatio)
                ->Field("VariableHeightEnabled", &JumpComponent::m_bVariableHeightEnabled)
                ->Field("JumpCutFactor", &JumpComponent::m_fJumpCutFactor)
                ->Field("CoyoteTimeEnabled", &JumpComponent::m_bCoyoteTimeEnabled)
                ->Field("CoyoteTime", &JumpComponent::m_fCoyoteTime)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<JumpComponent>("Jump",
                    "Makes the character jump. Owns the launch, the ceiling check and the rules for "
                    "jumping out of a crouch. Requires a Movement component for ground detection, and "
                    "the PhysX Character Controller and Character Gameplay components for the physics.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Modular Character Controller/Abilities")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))

                    ->DataElement(AZ::Edit::UIHandlers::Default, &JumpComponent::m_fJumpSpeed,
                        "Jump Speed",
                        "Upward speed applied at launch, in metres per second. This is a speed, not a "
                        "height: the apex works out to roughly speed squared over twice gravity, so "
                        "5 m/s reaches about 1.27 m under standard gravity.")

                    ->DataElement(AZ::Edit::UIHandlers::ComboBox, &JumpComponent::m_eCrouchJumpMode,
                        "Crouch Jump Mode",
                        "What happens when the jump key is pressed while crouched. Has no effect on a "
                        "standing character, or on an entity with no Crouch component.")
                    ->EnumAttribute(CrouchJumpMode::Forbid,
                        "Forbid - no jumping while crouched")
                    ->EnumAttribute(CrouchJumpMode::JumpCrouched,
                        "Jump crouched - stay low and jump anyway")
                    ->EnumAttribute(CrouchJumpMode::StandUpThenJump,
                        "Stand up, then jump - refused if a ceiling blocks standing")

                    ->DataElement(AZ::Edit::UIHandlers::Default, &JumpComponent::m_fJumpBufferTime,
                        "Jump Buffer Time",
                        "How long a jump press stays valid when it cannot be honoured yet, in seconds. "
                        "Press just before landing and the jump fires on touchdown instead of being "
                        "dropped. Around 0.1-0.15 reads as responsive; past 0.2 the character starts "
                        "jumping on its own.")

                    ->DataElement(AZ::Edit::UIHandlers::Default, &JumpComponent::m_fHeadHitMinStallTime,
                        "Head Hit Min Stall",
                        "Shortest ceiling stall worth reacting to, in seconds. Multiplied by scene gravity this "
                        "becomes the upward speed below which head hits are ignored - at 0.1 and standard gravity "
                        "that is about 1 m/s, and anything slower stalls for too little time to be seen.")

                    ->DataElement(AZ::Edit::UIHandlers::Default, &JumpComponent::m_fHeadHitStallRatio,
                        "Head Hit Stall Ratio",
                        "How much of the requested climb must survive before the jump is treated as blocked. At "
                        "0.5 the character must lose more than half its upward motion. Free flight normally keeps "
                        "about 95 percent, so there is a wide gap between the two.")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Variable Height")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &JumpComponent::m_bVariableHeightEnabled,
                        "Enable Variable Height",
                        "Cut the jump short when the key is released early, so a tap gives a lower hop "
                        "than a held press.")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::AttributesAndValues)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &JumpComponent::m_fJumpCutFactor,
                        "Jump Cut Factor",
                        "What fraction of the upward speed survives an early release. 0.4 keeps 40 "
                        "percent; 1.0 disables the cut entirely.")
                    ->Attribute(AZ::Edit::Attributes::ReadOnly, &JumpComponent::isJumpCutReadOnly)

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Coyote Time")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &JumpComponent::m_bCoyoteTimeEnabled,
                        "Enable Coyote Time",
                        "Still allow a jump for a short moment after walking off a ledge. Does not "
                        "apply after leaving the ground by jumping, so it cannot become a double jump.")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::AttributesAndValues)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &JumpComponent::m_fCoyoteTime,
                        "Coyote Time",
                        "How long the window stays open after footing is lost, in seconds. At 0.12 and "
                        "a walk speed of 3.5 m/s the character is about 42 cm past the edge when the "
                        "jump still counts.")
                    ->Attribute(AZ::Edit::Attributes::ReadOnly, &JumpComponent::isCoyoteTimeReadOnly)
                    ;
            }
        }
    }


    void JumpComponent::Init()
    {
        // Built once per component lifetime - the callback never changes between activations.
        // The event carries (SceneHandle, gravity); the scene is always ours, so the first
        // parameter is ignored.
        m_onGravityChangedHandler = AzPhysics::SceneEvents::OnSceneGravityChangedEvent::Handler(
            [this]([[maybe_unused]] AzPhysics::SceneHandle sceneHandle, const AZ::Vector3& newGravity)
            {
                m_gravity = newGravity;
            });
    }

    void JumpComponent::Activate()
    {
        JumpRequestBus::Handler::BusConnect(GetEntityId());
        AZ::TickBus::Handler::BusConnect();

        StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(JumpEventId);

        SetupSceneGravity();
    }

    void JumpComponent::Deactivate()
    {
        JumpRequestBus::Handler::BusDisconnect(GetEntityId());
        AZ::TickBus::Handler::BusDisconnect();

        StartingPointInput::InputEventNotificationBus::MultiHandler::BusDisconnect();

        m_onGravityChangedHandler.Disconnect();
    }


    void JumpComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("JumpComponentService"));
    }

    void JumpComponent::GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("JumpComponentService"));
    }

    void JumpComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("MovementComponentService"));
        // InputConfigurationService is required for the character's input handling
        required.push_back(AZ_CRC_CE("InputConfigurationService"));
        // PhysicsCharacterControllerService is required for the character's physics-based movement
        required.push_back(AZ_CRC_CE("PhysicsCharacterControllerService"));
        required.push_back(AZ_CRC_CE("PhysicsCharacterGameplayService"));
    }

    void JumpComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }



    void JumpComponent::OnPressed([[maybe_unused]] float value)
    {
        m_fTimeSinceJumpPressed = 0.0f;
    }

    void JumpComponent::OnHeld([[maybe_unused]] float value)
    {
    }

    void JumpComponent::OnReleased([[maybe_unused]] float value)
    {
        if (!m_bVariableHeightEnabled)
        {
            return;
        }

        AZ::Vector3 fallingVelocity = AZ::Vector3::CreateZero();
        PhysX::CharacterGameplayRequestBus::EventResult(
            fallingVelocity, GetEntityId(), &PhysX::CharacterGameplayRequests::GetFallingVelocity);

        // Only a climb can be cut short. Scaling a negative value would slow the fall instead,
        // so every mid-air key release would turn into a short glide.
        if (fallingVelocity.GetZ() <= 0.0f)
        {
            return;
        }

        // Keep X and Y untouched - the cut is about how much of the climb survives, nothing else.
        fallingVelocity.SetZ(fallingVelocity.GetZ() * m_fJumpCutFactor);

        PhysX::CharacterGameplayRequestBus::Event(
            GetEntityId(), &PhysX::CharacterGameplayRequests::SetFallingVelocity, fallingVelocity);
    }



    void JumpComponent::OnTick(float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        // Refresh the world state everything below reads. One query per frame means every
        // decision in this tick sees the same answer about the ground.
        MovementRequestBus::EventResult(m_bIsGrounded, GetEntityId(), &MovementRequests::IsGrounded);

        if (m_bIsGrounded)
        {
            // The coyote window is measured from the moment footing is lost, so while we still
            // have it the counter stays at zero rather than freezing at its last value.
            m_fTimeSinceGrounded = 0.0f;
            m_bIsJumped = false;
        }
        else
        {
            // deltaTime is real seconds since the previous tick, so the total stays correct
            // whatever the frame rate does.
            m_fTimeSinceGrounded += deltaTime;
        }

        m_fTimeSinceJumpPressed += deltaTime;

        // A buffered press is spent only when a jump actually happens, so a press made in
        // mid-air stays alive until the character lands - that is what the buffer is for.
        const bool jumpRequested = m_fTimeSinceJumpPressed <= m_fJumpBufferTime;

        if (CheckHeadHit())
        {
            PhysX::CharacterGameplayRequestBus::Event(
                GetEntityId(), &PhysX::CharacterGameplayRequests::SetFallingVelocity, AZ::Vector3::CreateZero());
        }

        if (jumpRequested && CanJumpNow() && ResolveCrouch())
        {
            DoJump();
            m_bIsJumped = true;
            m_fTimeSinceJumpPressed = NoBufferedJump;
        }
    }

    int JumpComponent::GetTickOrder()
    {
        return AZ::ComponentTickBus::TICK_GAME;
    }



    bool JumpComponent::CanJumpNow() const
    {
        // A jump already in flight blocks everything below, the coyote window included.
        // Without this, leaving the ground by jumping would open the window and hand out
        // a second jump in mid-air.
        if (m_bIsJumped)
        {
            return false;
        }

        if (m_bIsGrounded)
        {
            return true;
        }

        // Airborne: the only remaining way in is a coyote window that is still open.
        return m_bCoyoteTimeEnabled && m_fTimeSinceGrounded <= m_fCoyoteTime;
    }

    bool JumpComponent::ResolveCrouch()
    {
        bool isCrouching = false;
        CrouchRequestBus::EventResult(isCrouching, GetEntityId(), &CrouchRequests::IsCrouching);

        // The crouch-jump modes only describe behaviour while crouched. A standing character -
        // or one on an entity with no CrouchComponent, where the call above leaves isCrouching
        // untouched - is never restricted by them.
        if (!isCrouching)
        {
            return true;
        }

        switch (m_eCrouchJumpMode)
            {
            case CrouchJumpMode::Forbid:
                return false;

            case CrouchJumpMode::JumpCrouched:
                return true;

            case CrouchJumpMode::StandUpThenJump:
            {
                // TryStandUpOnce is a command, not a query: it stands the character up when there
                // is headroom and reports whether it did. One call covers both the check and the
                // transition, so no separate CanStandUp call is needed here.
                bool stoodUp = false;
                CrouchRequestBus::EventResult(stoodUp, GetEntityId(), &CrouchRequests::TryStandUpOnce);

                // Blocked by a ceiling means the mode's precondition was not met, so the jump is
                // refused. Return true instead to fall back to a crouched jump.
                return stoodUp;
            }
        }

        // Every enumerator is handled above and there is deliberately no default label, so adding
        // a new mode surfaces as a compiler warning rather than silently landing here.
        return true;
    }    

    void JumpComponent::DoJump()
    {
        if (!CanJumpNow()) return;

        Physics::CharacterRequestBus::Event(
            GetEntityId(), &Physics::CharacterRequests::AddVelocityForPhysicsTimestep, AZ::Vector3(0.0f, 0.0f, m_fJumpSpeed));

        PhysX::CharacterGameplayRequestBus::Event(
            GetEntityId(), &PhysX::CharacterGameplayRequests::SetFallingVelocity, AZ::Vector3(0.0f, 0.0f, m_fJumpSpeed));
    }

    bool JumpComponent::CheckHeadHit() const
    {
        // What the engine thinks our vertical speed is - this is the number that actually drives
        // the climb, so it is also what we expected to get out of this frame.
        AZ::Vector3 fallingVelocity = AZ::Vector3::CreateZero();
        PhysX::CharacterGameplayRequestBus::EventResult(
            fallingVelocity, GetEntityId(), &PhysX::CharacterGameplayRequests::GetFallingVelocity);

        const float expectedSpeed = fallingVelocity.GetZ();

        // Below this there is nothing worth catching: an undetected stall lasts expectedSpeed
        // divided by gravity, and under m_fHeadHitMinStallTime no player notices it. Deriving the
        // gate from scene gravity keeps it right on a low-gravity level instead of hardcoding a
        // number that only holds on Earth.
        const float gateSpeed = m_gravity.GetLength() * m_fHeadHitMinStallTime;
        if (expectedSpeed <= gateSpeed)
        {
            return false;
        }

        // What actually happened. Move() computes this as (newPosition - oldPosition) / deltaTime,
        // so it is the outcome of the collision rather than a guess about it - no geometry to keep
        // in sync, no collision group to match.
        AZ::Vector3 observedVelocity = AZ::Vector3::CreateZero();
        Physics::CharacterRequestBus::EventResult(
            observedVelocity, GetEntityId(), &Physics::CharacterRequests::GetVelocity);

        AZ_Printf("JumpComponent", "head: expected=%.3f observed=%.3f ratio=%.3f gate=%.3f",
            expectedSpeed, observedVelocity.GetZ(),
            expectedSpeed > 0.0f ? observedVelocity.GetZ() / expectedSpeed : 0.0f, gateSpeed);

        // In free flight the two differ by exactly one substep of gravity, so the ratio sits around
        // 0.95 and never approaches the threshold. Losing more than half the requested climb means
        // something solid stopped us.
        return observedVelocity.GetZ() < expectedSpeed * m_fHeadHitStallRatio;
    }

    void JumpComponent::SetupSceneGravity()
    {
        AzPhysics::SceneHandle sceneHandle = AzPhysics::InvalidSceneHandle;
        Physics::DefaultWorldBus::BroadcastResult(sceneHandle,
            &Physics::DefaultWorldRequests::GetDefaultSceneHandle);

        if (sceneHandle == AzPhysics::InvalidSceneHandle)
        {
            AZ_Error("JumpComponent", false, "No default physics scene available.");
            return;
        }

        if (auto* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get())
        {
            // Current value...
            m_gravity = sceneInterface->GetGravity(sceneHandle);
            // ...and every change from now on.
            sceneInterface->RegisterSceneGravityChangedEvent(sceneHandle, m_onGravityChangedHandler);
        }
    }
    
} // namespace ModularCharacterController
