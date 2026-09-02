#pragma once

#include <AzCore/Component/Component.h>
#include <ModularCharacterController/CrouchInterface.h>
#include <ModularCharacterController/MovementInterface.h>

#include <StartingPointInput/InputEventNotificationBus.h>

#include <AzCore/Component/TickBus.h>

#include <AzCore/Debug/Trace.h>

#include <AzFramework/Physics/PhysicsSystem.h>
#include <AzFramework/Physics/PhysicsScene.h>
#include <AzFramework/Physics/Common/PhysicsSceneQueries.h>
#include <AzFramework/Physics/Collision/CollisionGroups.h>

namespace ModularCharacterController
{
    class CrouchComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
        , public CrouchRequestBus::Handler
        , public StartingPointInput::InputEventNotificationBus::MultiHandler
    {
    public:
        AZ_COMPONENT_DECL(CrouchComponent);

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

    protected:
        void Activate() override;
        void Deactivate() override;

        // Input
        void OnPressed([[maybe_unused]] float value) override;
        void OnHeld([[maybe_unused]] float value) override;
        void OnReleased([[maybe_unused]] float value) override;

        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

    private:
        float m_fInitialCapsuleHeight = 0.0f;
        float m_fAppliedCrouchHeight = 0.0f;
        float m_fInitialCapsuleRadius = 0.0f;

        float m_fCrouchHeight = 1.0f;
        float m_fStandUpSafetyMargin = 0.1f;
        float m_fCrouchSpeedScale = 0.5f;
        bool m_bToggleMode = true;

        bool m_bIsCrouching = false;
        bool m_bWantsToStand = false;

        void EnterCrouch();
        void TryStandUp();
        bool CanStandUp() const;

        bool IsCrouching() const override { return m_bIsCrouching; }

        void RefreshCapsuleDimensions();

        // Input events
        inline static const StartingPointInput::InputEventNotificationId CrouchEventId{ "Crouch" };
    };
} // namespace ModularCharacterController
