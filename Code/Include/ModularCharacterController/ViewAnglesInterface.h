
#pragma once

#include <AzCore/Component/ComponentBus.h>

namespace ModularCharacterController
{
    class ViewAnglesRequests
        : public AZ::ComponentBus
    {
    public:
        AZ_RTTI(ModularCharacterController::ViewAnglesRequests, "{295C2AB0-1E84-4711-ADCA-24EE4ED36FA8}");

        // Look direction in degrees. Yaw is wrapped to [-180, 180] and is unbounded in
        // meaning; pitch is clamped to the configured limit and is the value a view
        // component applies to its own camera.
        virtual float GetYawDegrees() const = 0;
        virtual float GetPitchDegrees() const = 0;
        
    };

    using ViewAnglesRequestBus = AZ::EBus<ViewAnglesRequests>;

} // namespace ModularCharacterController
