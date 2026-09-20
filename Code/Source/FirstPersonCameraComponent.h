
#pragma once

#include <AzCore/Component/Component.h>
#include <ModularCharacterController/FirstPersonCameraInterface.h>

#include <AzCore/Component/TickBus.h>

namespace ModularCharacterController
{
    class FirstPersonCameraComponent
        : public AZ::Component
        , public FirstPersonCameraRequestBus::Handler
        , protected AZ::TickBus::Handler
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

        void OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time) override;
        int  GetTickOrder() override;

    private:
        AZ::EntityId m_cameraEntityId;
    };
} // namespace ModularCharacterController
