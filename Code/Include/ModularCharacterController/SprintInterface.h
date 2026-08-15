
#pragma once

#include <AzCore/Component/ComponentBus.h>

namespace ModularCharacterController
{
    class SprintRequests
        : public AZ::ComponentBus
    {
    public:
        AZ_RTTI(ModularCharacterController::SprintRequests, "{23D1A8E6-AC49-49CA-B60D-0286EC993A47}");

        // Put your public request methods here.
        
        // Put notification events here. Examples:
        // void RegisterEvent(AZ::EventHandler<...> notifyHandler);
        // AZ::Event<...> m_notifyEvent1;
        
    };

    using SprintRequestBus = AZ::EBus<SprintRequests>;

} // namespace ModularCharacterController
