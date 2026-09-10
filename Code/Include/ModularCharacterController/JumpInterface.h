
#pragma once

#include <AzCore/Component/ComponentBus.h>

namespace ModularCharacterController
{
    class JumpRequests
        : public AZ::ComponentBus
    {
    public:
        AZ_RTTI(ModularCharacterController::JumpRequests, "{58EC7F8B-8B29-4E5C-B023-EECDB1D7B7A8}");
    };

    using JumpRequestBus = AZ::EBus<JumpRequests>;

} // namespace ModularCharacterController
