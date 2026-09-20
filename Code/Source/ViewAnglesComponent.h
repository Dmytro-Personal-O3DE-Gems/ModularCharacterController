
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

        void OnTick(float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time) override;
        int  GetTickOrder() override;

        void OnPressed(float value) override;
        void OnHeld(float value) override;
        void OnReleased([[maybe_unused]] float value) override;

    private:
        // Interface methods
        float GetYawDegrees() const override { return m_fYawDegrees; }
        float GetPitchDegrees() const override { return m_fPitchDegrees; }

        // The look direction itself. This component owns both values; the body rotation
        // is derived from the yaw, and the camera pitch is applied by a view component
        // that reads GetPitchDegrees().
        float m_fYawDegrees = 0.0f;
        float m_fPitchDegrees = 0.0f;

        // Input carried from the input callbacks to OnTick. The two devices need opposite
        // handling, which is why they cannot share a field:
        //   mouse - increments, they add up and are consumed once per tick;
        //   stick - a level in [-1, 1], the latest value wins and survives until release.
        float m_fMouseDeltaX = 0.0f;
        float m_fMouseDeltaY = 0.0f;
        float m_fStickX = 0.0f;
        float m_fStickY = 0.0f;

        // Degrees per unit of mouse delta.
        float m_fMouseSensitivityX = 0.1f;
        float m_fMouseSensitivityY = 0.1f;

        // Degrees per second at full stick deflection. Three orders of magnitude away from
        // the mouse values above because the unit is different, not because it is faster.
        float m_fStickSensitivityX = 150.0f;
        float m_fStickSensitivityY = 120.0f;

        // Vertical inversion is a motor habit, not a taste: a player used to it cannot
        // play without it. Kept separate per device because wanting it on a pad and not
        // on a mouse is common.
        bool m_bInvertMouseY = false;
        bool m_bInvertStickY = false;

        // Stops just short of 90 degrees on purpose: at exactly 90 the forward direction
        // becomes degenerate and any code deriving a heading from the look direction
        // loses its horizontal component entirely.
        float m_fMaxPitchDegrees = 89.0f;

        bool ShouldBodyFollowYaw() const;
        void ApplyBodyYaw() const;

        // Input events
        inline static const StartingPointInput::InputEventNotificationId LookMouseXEventId{ "LookMouseX" };
        inline static const StartingPointInput::InputEventNotificationId LookMouseYEventId{ "LookMouseY" };
        inline static const StartingPointInput::InputEventNotificationId LookStickXEventId{ "LookStickX" };
        inline static const StartingPointInput::InputEventNotificationId LookStickYEventId{ "LookStickY" };
    };
} // namespace ModularCharacterController
