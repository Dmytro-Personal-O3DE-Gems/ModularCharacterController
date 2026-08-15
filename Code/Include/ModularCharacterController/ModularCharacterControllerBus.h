
#pragma once

#include <ModularCharacterController/ModularCharacterControllerTypeIds.h>

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace ModularCharacterController
{
    class ModularCharacterControllerRequests
    {
    public:
        AZ_RTTI(ModularCharacterControllerRequests, ModularCharacterControllerRequestsTypeId);
        virtual ~ModularCharacterControllerRequests() = default;
        // Put your public methods here
    };

    class ModularCharacterControllerBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using ModularCharacterControllerRequestBus = AZ::EBus<ModularCharacterControllerRequests, ModularCharacterControllerBusTraits>;
    using ModularCharacterControllerInterface = AZ::Interface<ModularCharacterControllerRequests>;

} // namespace ModularCharacterController
