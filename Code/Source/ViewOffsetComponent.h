
#pragma once

#include <AzCore/Component/Component.h>
#include <ModularCharacterController/ViewOffsetInterface.h>
#include <ModularCharacterController/MovementInterface.h>
#include <ModularCharacterController/GroundTrackerInterface.h>

#include <AzFramework/Physics/CharacterBus.h> 
#include <AzCore/Component/TickBus.h>

#include <AzCore/Math/Vector3.h>
#include <AzCore/Math/Crc.h>
#include <AzCore/std/containers/fixed_vector.h>

namespace ModularCharacterController
{
    class ViewOffsetComponent
        : public AZ::Component
        , public ViewOffsetRequestBus::Handler
        , public GroundTrackerNotificationBus::Handler
        , protected AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT_DECL(ViewOffsetComponent);

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        void OnLanded(const LandingInfo& landing) override;

    protected:
        void Activate() override;
        void Deactivate() override;

        void OnTick(float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time) override;

        void SetOffset(AZ::Crc32 channel, const AZ::Vector3& offset) override;
        AZ::Vector3 GetOffset() const override { return m_vTotalOffset; }

    private:
        // Assembled once per tick from the fields below. Assignment, never accumulation:
        // it remembers nothing between frames, so nothing can corrupt it.
        AZ::Vector3 m_vTotalOffset = AZ::Vector3::CreateZero();

        // --- Crouch -----------------------------------------------------------------
        float m_fCrouchOffsetZ = 0.0f;        // Current smoothed drop from the standing height.
        float m_fCrouchSmoothingRate = 10.0f; // How fast the view catches up to a height change.

        // --- Head bob ---------------------------------------------------------------
        bool  m_bEnableBob = true;
        AZ::Vector3 m_vBobOffset = AZ::Vector3::CreateZero(); // This effect's own contribution.
        float m_fBobPhase = 0.0f;             // Radians, advanced by distance covered, wrapped at 2*pi.
        float m_fBobWeight = 0.0f;            // 0..1. How much bob is showing right now.
        float m_fBobBlendRate = 8.0f;         // How fast the weight follows the conditions that drive it.
        float m_fBobStrideLength = 1.6f;      // Metres per full gait cycle.
        float m_fBobReferenceSpeed = 3.5f;    // Speed at which the amplitudes below are reached.
        float m_fBobAmplitudeUp = 0.02f;      // Vertical travel of the eye, in metres.
        float m_fBobAmplitudeSide = 0.01f;    // Sideways travel of the eye, in metres.

        // --- Landing dip ------------------------------------------------------------
        bool  m_bEnableLandingDip = true;
        float m_fLandingDipZ = 0.0f;             // Current drop caused by an impact, in metres. Negative.
        float m_fPendingImpactSpeed = 0.0f;      // Armed by OnLanded, consumed by the next tick. 0 means nothing pending.
        float m_fLandingDipMinSpeed = 2.0f;      // Impacts slower than this produce no dip at all.
        float m_fLandingDipMaxSpeed = 4.0f;      // At this speed and above the dip reaches its full depth.
        float m_fLandingDipMaxDepth = 0.10f;     // Depth at full strength, in metres. Also the cap on stacked impacts.
        float m_fLandingDipRecoveryRate = 8.0f;  // How fast the eye returns to neutral afterwards.

        void UpdateCrouchOffset(float deltaTime);
        void UpdateBobOffset(float deltaTime);
        void UpdateLandingDip(float deltaTime);
    };
} // namespace ModularCharacterController
