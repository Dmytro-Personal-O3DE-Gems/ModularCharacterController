
#pragma once

#include <AzCore/Component/ComponentBus.h>

#include <AzCore/Math/Vector3.h>

namespace ModularCharacterController
{
    class FirstPersonCameraRequests
        : public AZ::ComponentBus
    {
    public:
        AZ_RTTI(ModularCharacterController::FirstPersonCameraRequests, "{C9B879D1-EADB-47D5-9E64-1616452E1E25}");

        // Intentionally empty for now. The camera's own base position stays private:
        // ViewOffset returns a pure delta and must not know where the camera sits.
    };

    using FirstPersonCameraRequestBus = AZ::EBus<FirstPersonCameraRequests>;

} // namespace ModularCharacterController
