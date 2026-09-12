
#pragma once

#include <AzCore/Component/ComponentBus.h>

namespace ModularCharacterController
{
    class FirstPersonCameraRequests
        : public AZ::ComponentBus
    {
    public:
        AZ_RTTI(ModularCharacterController::FirstPersonCameraRequests, "{C9B879D1-EADB-47D5-9E64-1616452E1E25}");

        // Put your public request methods here.
        
        // Put notification events here. Examples:
        // void RegisterEvent(AZ::EventHandler<...> notifyHandler);
        // AZ::Event<...> m_notifyEvent1;
        
    };

    using FirstPersonCameraRequestBus = AZ::EBus<FirstPersonCameraRequests>;

} // namespace ModularCharacterController
