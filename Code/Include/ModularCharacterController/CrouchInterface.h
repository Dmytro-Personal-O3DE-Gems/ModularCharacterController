
#pragma once

#include <AzCore/Component/ComponentBus.h>

namespace ModularCharacterController
{
    class CrouchRequests
        : public AZ::ComponentBus
    {
    public:
<<<<<<< Updated upstream
        AZ_RTTI(ModularCharacterController::CrouchRequests, "{6C68BE31-0D8D-4872-A1A3-21083A863B0C}");
=======
        AZ_RTTI(ModularCharacterController::CrouchRequests, "{68070006-0A35-4CC5-BF7F-8EB1BD079A2F}");
>>>>>>> Stashed changes

        // Put your public request methods here.
        
        // Put notification events here. Examples:
        // void RegisterEvent(AZ::EventHandler<...> notifyHandler);
        // AZ::Event<...> m_notifyEvent1;
        
<<<<<<< Updated upstream
=======
        virtual bool IsCrouching() const = 0;
>>>>>>> Stashed changes
    };

    using CrouchRequestBus = AZ::EBus<CrouchRequests>;

} // namespace ModularCharacterController
