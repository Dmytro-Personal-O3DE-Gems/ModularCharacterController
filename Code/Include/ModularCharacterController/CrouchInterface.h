#pragma once

#include <AzCore/Component/ComponentBus.h>

namespace ModularCharacterController
{
    class CrouchRequests
        : public AZ::ComponentBus
    {
    public:
        AZ_RTTI(ModularCharacterController::CrouchRequests, "{68070006-0A35-4CC5-BF7F-8EB1BD079A2F}");

        virtual bool IsCrouching() const = 0;
        virtual bool TryStandUpOnce() = 0;
    };

    using CrouchRequestBus = AZ::EBus<CrouchRequests>;

} // namespace ModularCharacterController
