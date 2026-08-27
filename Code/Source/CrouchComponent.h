<<<<<<< Updated upstream

=======
﻿
>>>>>>> Stashed changes
#pragma once

#include <AzCore/Component/Component.h>
#include <ModularCharacterController/CrouchInterface.h>
<<<<<<< Updated upstream

// Cast
#include <AzFramework/Physics/PhysicsSystem.h>             // AzPhysics::SystemInterface — GetSceneHandle / GetScene
#include <AzFramework/Physics/PhysicsScene.h>               // AzPhysics::Scene — method QueryScene(request)
#include <AzFramework/Physics/Common/PhysicsSceneQueries.h> // RayCastRequest / ShapeCastRequest / OverlapRequest / SceneQueryHits / *RequestHelpers
#include <AzFramework/Physics/Collision/CollisionGroups.h>  // CollisionGroup
=======
#include <ModularCharacterController/MovementInterface.h>

#include <StartingPointInput/InputEventNotificationBus.h>

#include <AzCore/Component/TickBus.h>

#include <AzCore/Debug/Trace.h>

#include <AzFramework/Physics/PhysicsSystem.h>
#include <AzFramework/Physics/PhysicsScene.h>
#include <AzFramework/Physics/Common/PhysicsSceneQueries.h>
#include <AzFramework/Physics/Collision/CollisionGroups.h>
>>>>>>> Stashed changes

namespace ModularCharacterController
{
    class CrouchComponent
        : public AZ::Component
<<<<<<< Updated upstream
        , public CrouchRequestBus::Handler
=======
        , public AZ::TickBus::Handler
        , public CrouchRequestBus::Handler
        , public StartingPointInput::InputEventNotificationBus::MultiHandler
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
    };
} // namespace ModularCharacterController
=======

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
        float m_fStandUpSafetyMargin = 0.05f;
        bool m_bToggleMode = true;

        bool m_bIsCrouching = false;
        bool m_bWantsToStand = false;

        void EnterCrouch();
        void TryStandUp();       // TODO: выставить m_bWantsToStand, подключить TickBus
        bool CanStandUp() const;

        bool IsCrouching() const override { return m_bIsCrouching; }


        void RefreshCapsuleDimensions();

        // Input events
        inline static const StartingPointInput::InputEventNotificationId CrouchEventId{ "Crouch" };
    };
} // namespace ModularCharacterController
>>>>>>> Stashed changes
