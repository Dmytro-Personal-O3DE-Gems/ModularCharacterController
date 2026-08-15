
#pragma once

#include <AzCore/Component/ComponentBus.h>

namespace ModularCharacterController
{
    class CrouchRequests
        : public AZ::ComponentBus
    {
    public:
        AZ_RTTI(ModularCharacterController::CrouchRequests, "{6C68BE31-0D8D-4872-A1A3-21083A863B0C}");

        // Put your public request methods here.
        
        // Put notification events here. Examples:
        // void RegisterEvent(AZ::EventHandler<...> notifyHandler);
        // AZ::Event<...> m_notifyEvent1;
        
    };

    using CrouchRequestBus = AZ::EBus<CrouchRequests>;

} // namespace ModularCharacterController
