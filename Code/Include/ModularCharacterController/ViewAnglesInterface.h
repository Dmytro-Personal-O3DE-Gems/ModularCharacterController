
#pragma once

#include <AzCore/Component/ComponentBus.h>

namespace ModularCharacterController
{
    class ViewAnglesRequests
        : public AZ::ComponentBus
    {
    public:
        AZ_RTTI(ModularCharacterController::ViewAnglesRequests, "{295C2AB0-1E84-4711-ADCA-24EE4ED36FA8}");

        // Put your public request methods here.
        
        // Put notification events here. Examples:
        // void RegisterEvent(AZ::EventHandler<...> notifyHandler);
        // AZ::Event<...> m_notifyEvent1;
        
    };

    using ViewAnglesRequestBus = AZ::EBus<ViewAnglesRequests>;

} // namespace ModularCharacterController
