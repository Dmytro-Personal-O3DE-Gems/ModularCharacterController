
#pragma once

#include <AzCore/Component/Component.h>
#include <ModularCharacterController/ViewAnglesInterface.h>

#include <AzCore/Component/TickBus.h>

// Input
#include <StartingPointInput/InputEventNotificationBus.h>

namespace ModularCharacterController
{
    class ViewAnglesComponent
        : public AZ::Component
        , public ViewAnglesRequestBus::Handler
        , protected AZ::TickBus::Handler
        , protected StartingPointInput::InputEventNotificationBus::MultiHandler
    {
    public:
        AZ_COMPONENT_DECL(ViewAnglesComponent);

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

        void OnPressed([[maybe_unused]] float value) override;
        void OnHeld([[maybe_unused]] float value) override;
        void OnReleased([[maybe_unused]] float value) override;

    private:
        float m_fYawDegrees = 0.0f;
        float m_fPitchDegrees = 0.0f;

        float m_fMouseSensitivityX = 0.1f;
        float m_fMouseSensitivityY = 0.1f;
        float m_fStickSensitivityX = 180.0f;
        float m_fStickSensitivityY = 180.0f;

        float m_fMaxPitchDegrees = 89.0f;

        inline static const StartingPointInput::InputEventNotificationId LookMouseXEventId{ "LookMouseX" };
        inline static const StartingPointInput::InputEventNotificationId LookMouseYEventId{ "LookMouseY" };
        inline static const StartingPointInput::InputEventNotificationId LookStickXEventId{ "LookStickX" };
        inline static const StartingPointInput::InputEventNotificationId LookStickYEventId{ "LookStickY" };
    };
} // namespace ModularCharacterController
