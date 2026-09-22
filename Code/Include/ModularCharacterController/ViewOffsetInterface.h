
#pragma once

#include <AzCore/Component/ComponentBus.h>

namespace ModularCharacterController
{
    class ViewOffsetRequests
        : public AZ::ComponentBus
    {
    public:
        AZ_RTTI(ModularCharacterController::ViewOffsetRequests, "{E8587510-494C-4F12-AA9A-F4B4DE216F45}");

        // Put your public request methods here.
        
        // Put notification events here. Examples:
        // void RegisterEvent(AZ::EventHandler<...> notifyHandler);
        // AZ::Event<...> m_notifyEvent1;
        
    };

    using ViewOffsetRequestBus = AZ::EBus<ViewOffsetRequests>;

} // namespace ModularCharacterController
