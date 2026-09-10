
#pragma once

#include <AzCore/Component/Component.h>
#include <ModularCharacterController/JumpInterface.h>
#include <ModularCharacterController/MovementInterface.h>
#include <ModularCharacterController/CrouchInterface.h>

#include <AzCore/Component/TickBus.h>

// Input
#include <StartingPointInput/InputEventNotificationBus.h>

// Math
#include <AzCore/Math/Vector3.h>          // AZ::Vector3

// Physics
#include <AzFramework/Physics/PhysicsSystem.h>  // AzPhysics::SceneInterface
#include <AzFramework/Physics/PhysicsScene.h>   // AzPhysics::SceneEvents
#include <AzFramework/Physics/SystemBus.h>      // Physics::DefaultWorldBus
#include <AzFramework/Physics/Character.h>      // Physics::AddVelocityForTick

// Cast
#include <AzFramework/Physics/PhysicsSystem.h>              // AzPhysics::SystemInterface — GetSceneHandle / GetScene
#include <AzFramework/Physics/PhysicsScene.h>               // AzPhysics::Scene — сам метод QueryScene(request)
#include <AzFramework/Physics/Common/PhysicsSceneQueries.h> // RayCastRequest / ShapeCastRequest / OverlapRequest / SceneQueryHits / *RequestHelpers
#include <AzFramework/Physics/Collision/CollisionGroups.h>  // CollisionGroup

#include <PhysX/CharacterGameplayBus.h>

namespace ModularCharacterController
{
    enum class CrouchJumpMode : AZ::u8
    {
        Forbid,
        JumpCrouched,
        StandUpThenJump
    };

    class JumpComponent
        : public AZ::Component
        , public JumpRequestBus::Handler
        , protected AZ::TickBus::Handler
        , protected StartingPointInput::InputEventNotificationBus::MultiHandler
    {
    public:
        AZ_COMPONENT_DECL(JumpComponent);

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

    protected:
        void Init() override;
        void Activate() override;
        void Deactivate() override;

        void OnPressed(float value) override;
        void OnHeld(float value) override;
        void OnReleased(float value) override;

        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        int  GetTickOrder() override;

    private:
        // Settings
        float m_fJumpSpeed = 2.5f;
        AZ::Vector3 m_gravity = AZ::Vector3::CreateZero();
        CrouchJumpMode m_eCrouchJumpMode = CrouchJumpMode::StandUpThenJump;

        bool  m_bVariableHeightEnabled = false;
        float m_fJumpCutFactor = 0.4f;
        bool  isJumpCutReadOnly() const { return !m_bVariableHeightEnabled; }

        bool  m_bCoyoteTimeEnabled = false;
        float m_fCoyoteTime = 0.12f;
        bool  isCoyoteTimeReadOnly() const { return !m_bCoyoteTimeEnabled; }



        // Shortest ceiling stall a player can notice, in seconds. Multiplied by scene gravity it
        // gives the speed below which a head hit is not worth detecting.
        float m_fHeadHitMinStallTime = 0.1f;

        // How much of the requested climb must survive before we call it a hit.
        float m_fHeadHitStallRatio = 0.5f;
        


        // State
        bool m_bIsJumped = false;
        bool m_bIsGrounded = false;
        float m_fTimeSinceGrounded = 0.0f;
        // A press older than this window is treated as absent. The field starts at the sentinel
        // and returns to it once a press has been spent.
        static constexpr float NoBufferedJump = 1000.0f;

        float m_fJumpBufferTime = 0.12f;
        float m_fTimeSinceJumpPressed = NoBufferedJump;

        bool CanJumpNow() const;
        bool ResolveCrouch();
        void DoJump();
        bool CheckHeadHit() const;

        float m_fCapsuleHeight = 0.0f;
        float m_fCapsuleRadius = 0.0f;

        inline static const StartingPointInput::InputEventNotificationId JumpEventId{ "Jump" };

        void SetupSceneGravity();

        AzPhysics::SceneEvents::OnSceneGravityChangedEvent::Handler m_onGravityChangedHandler;
    };
} // namespace ModularCharacterController
