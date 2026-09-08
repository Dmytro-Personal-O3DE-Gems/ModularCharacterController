
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

        bool IsAirborne() const override;

    private:
        // Settings
        float m_fJumpSpeed = 2.5f;
        AZ::Vector3 m_gravity = AZ::Vector3::CreateZero();
        CrouchJumpMode m_eCrouchJumpMode = CrouchJumpMode::StandUpThenJump;

        bool  m_bVariableHeight = false;
        float m_fJumpCutFactor = 0.4f;
        bool  isJumpCutReadOnly() const { return !m_bVariableHeight; }

        bool  m_bCoyoteTimeEnabled = false;
        float m_fCoyoteTime = 0.12f;
        bool  isCoyoteTimeReadOnly() const { return !m_bCoyoteTimeEnabled; }

        float m_fHeadHitCheckDistance = 0.20f;

        // State
        float m_fVerticalVelocity = 0.0f;
        float m_fTimeSinceGrounded = 0.0f;
        float m_fSavedGravityMultiplier = 1.0f;

        bool CanJumpNow() const;      // TODO: земля, либо coyote-окно
        bool ResolveCrouch();         // TODO: три режима; вернуть, разрешён ли прыжок
        void DoJump();                // TODO: одна строка — задать m_fVerticalVelocity
        bool CheckHeadHit() const;    // геометрия из CanStandUp, только вверх и на m_fHeadHitCheckDistance

        inline static const StartingPointInput::InputEventNotificationId JumpEventId{ "Jump" };

        void SetupSceneGravity();

        AzPhysics::SceneEvents::OnSceneGravityChangedEvent::Handler m_onGravityChangedHandler;
    };
} // namespace ModularCharacterController
