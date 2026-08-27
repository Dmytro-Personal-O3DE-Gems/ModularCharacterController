
#pragma once

#include <AzCore/Component/ComponentBus.h>

namespace ModularCharacterController
{
    class MovementRequests
        : public AZ::ComponentBus
    {
    public:
        AZ_RTTI(ModularCharacterController::MovementRequests, "{69B9E721-3195-4CC4-AC07-DC1160203BFF}");

        // Put your public request methods here.
        
        // Put notification events here. Examples:
        // void RegisterEvent(AZ::EventHandler<...> notifyHandler);
        // AZ::Event<...> m_notifyEvent1;
        
        virtual void SetSpeedMultiplier(float multiplier) = 0;
        virtual float GetForwardInput() const = 0;

        // Capsule
        virtual float GetStandingCapsuleHeight() const = 0;

        virtual float GetCapsuleHeight() const = 0;
        virtual float GetCapsuleRadius() const = 0;

        virtual void SetCapsuleHeight(float height) = 0;
        virtual void SetCapsuleRadius(float radius) = 0;
    };

    using MovementRequestBus = AZ::EBus<MovementRequests>;

} // namespace ModularCharacterController
