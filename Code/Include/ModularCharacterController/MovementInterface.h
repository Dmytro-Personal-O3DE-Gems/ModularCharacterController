
#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/Math/Crc.h>

#include <AzFramework/Physics/Collision/CollisionGroups.h>

namespace ModularCharacterController
{
    namespace SpeedChannels
    {
        inline constexpr AZ::Crc32 Crouch{ "Crouch" };
        inline constexpr AZ::Crc32 Sprint{ "Sprint" };
        inline constexpr AZ::Crc32 Jump{ "Jump" };
    }



    class MovementRequests
        : public AZ::ComponentBus
    {
    public:
        AZ_RTTI(ModularCharacterController::MovementRequests, "{69B9E721-3195-4CC4-AC07-DC1160203BFF}");
        
        //virtual void SetSpeedMultiplier(float multiplier) = 0;
        virtual void  SetSpeedScale(AZ::Crc32 channel, float scale) = 0;
        virtual float GetSpeedScale(AZ::Crc32 channel) const = 0;
        virtual float GetTotalSpeedScale() const = 0;
        
        virtual float GetForwardInput() const = 0;

        // Capsule
        virtual float GetStandingCapsuleHeight() const = 0;

        virtual float GetCapsuleHeight() const = 0;
        virtual float GetCapsuleRadius() const = 0;

        virtual void SetCapsuleHeight(float height) = 0;
        virtual void SetCapsuleRadius(float radius) = 0;

        //! What counts as solid for this character, ready to be handed to a scene query.
        //! Returned already resolved rather than as a CollisionGroups::Id on purpose: every
        //! lookup in that API silently falls back to CollisionGroup::All when the id is
        //! unknown, so resolving it per caller would widen each query without anyone noticing.
        //! One owner resolves, one owner warns.
        virtual AzPhysics::CollisionGroup GetSolidCollisionGroup() const = 0;
    };

    using MovementRequestBus = AZ::EBus<MovementRequests>;

} // namespace ModularCharacterController
