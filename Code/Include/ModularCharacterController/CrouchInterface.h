#pragma once

#include <AzCore/Component/ComponentBus.h>

namespace ModularCharacterController
{
    class CrouchRequests
        : public AZ::ComponentBus
    {
    public:
        AZ_RTTI(ModularCharacterController::CrouchRequests, "{68070006-0A35-4CC5-BF7F-8EB1BD079A2F}");

        // Put your public request methods here.

        // Put notification events here. Examples:
        // void RegisterEvent(AZ::EventHandler<...> notifyHandler);
        // AZ::Event<...> m_notifyEvent1;

        virtual bool IsCrouching() const = 0;
    };

    using CrouchRequestBus = AZ::EBus<CrouchRequests>;

} // namespace ModularCharacterController
