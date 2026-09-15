
#pragma once

#include <AzCore/Component/Component.h>
#include <ModularCharacterController/FirstPersonCameraInterface.h>

// Input
#include <StartingPointInput/InputEventNotificationBus.h>

namespace ModularCharacterController
{
    /*
    * TODO: Register this component in your Gem's AZ::Module interface by inserting the following into the list of m_descriptors:
    *       FirstPersonCameraComponent::CreateDescriptor(),
    */

    class FirstPersonCameraComponent
        : public AZ::Component
        , public FirstPersonCameraRequestBus::Handler
        , protected StartingPointInput::InputEventNotificationBus::MultiHandler
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

        void OnPressed(float value) override;
        void OnHeld(float value) override;
        void OnReleased(float value) override;

    private:
        AZ::EntityId m_cameraEntityId;

        float m_Yaw = 0.0f;
        float m_Pitch = 0.0f;

        AZ::Vector3 m_vInitialCameraTransform{ AZ::Vector3::CreateZero() };
        float m_fCrouchCameraHeight = 0.65f;

        float m_fMouseSensitivityX = 0.1f;
        float m_fMouseSensitivityY = 0.1f;

        float m_fMaxPitchDegrees = 89.0f;


        inline static const StartingPointInput::InputEventNotificationId YawEventId{ "Yaw" };
        inline static const StartingPointInput::InputEventNotificationId PitchEventId{ "Pitch" };
    };
} // namespace ModularCharacterController
