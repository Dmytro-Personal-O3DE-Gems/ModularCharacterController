
#pragma once

#include <AzCore/Component/Component.h>
#include <ModularCharacterController/FirstPersonCameraInterface.h>
#include <ModularCharacterController/ViewModeInterface.h>
#include <ModularCharacterController/ViewAnglesInterface.h>

#include <AzCore/Component/TickBus.h>
#include <AzCore/Component/EntityBus.h>
#include <AzFramework/Components/CameraBus.h>

namespace ModularCharacterController
{
    class FirstPersonCameraComponent
        : public AZ::Component
        , public FirstPersonCameraRequestBus::Handler
        , public ViewModeRequestBus::Handler
        , protected AZ::TickBus::Handler
        , protected AZ::EntityBus::Handler
        , protected Camera::CameraNotificationBus::Handler
    {
    public:
        AZ_COMPONENT_DECL(FirstPersonCameraComponent);
        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

    protected:
        void Activate() override;
        void Deactivate() override;

        void OnTick(float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time) override;
        int  GetTickOrder() override;

        // AZ::EntityBus - watching the camera entity, not this one.
        void OnEntityActivated(const AZ::EntityId& entityId) override;

        // Camera::CameraNotificationBus - broadcast, tells every listener which camera
        // became the active view.
        void OnActiveViewChanged(const AZ::EntityId& cameraEntityId) override;

    private:
        // ViewModeRequestBus. First person always turns the body with the look; an orbiting
        // third person view will answer false to this same question. ViewAngles asks, and
        // only the component whose camera is the active view is connected to answer.
        bool GetBodyFollowsYaw() const override { return true; }

        // The only place in the gem that knows which entity carries the camera.
        AZ::EntityId m_cameraEntityId;
    };
} // namespace ModularCharacterController
