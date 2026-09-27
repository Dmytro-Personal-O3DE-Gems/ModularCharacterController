
#pragma once

#include <AzCore/Component/ComponentBus.h>

#include <AzCore/Math/Vector3.h>
#include <AzCore/Math/Crc.h>

namespace ModularCharacterController
{
    // Channel names for EXTERNAL contributors go here - weapon recoil, explosion shake,
    // cutscenes. Crouch, head bob and the landing dip are built in: they are computed
    // inside ViewOffset with their own smoothing, and must not arrive through SetOffset.

    class ViewOffsetRequests
        : public AZ::ComponentBus
    {
    public:
        AZ_RTTI(ModularCharacterController::ViewOffsetRequests, "{E8587510-494C-4F12-AA9A-F4B4DE216F45}");

        virtual void SetOffset(AZ::Crc32 channel, const AZ::Vector3& offset) = 0;
        virtual AZ::Vector3 GetOffset() const = 0;
    };

    using ViewOffsetRequestBus = AZ::EBus<ViewOffsetRequests>;

} // namespace ModularCharacterController
