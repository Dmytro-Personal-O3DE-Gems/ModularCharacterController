
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
        , protected AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT_DECL(ViewOffsetComponent);

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

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
        float m_fBobStrideLength = 1.6f;      // Metres per full gait cycle.
        float m_fBobReferenceSpeed = 3.5f;    // Speed at which the amplitudes below are reached.
        float m_fBobAmplitudeUp = 0.02f;      // Vertical travel of the eye, in metres.
        float m_fBobAmplitudeSide = 0.01f;    // Sideways travel of the eye, in metres.

        void UpdateCrouchOffset(float deltaTime);
        void UpdateBobOffset(float deltaTime);
		void UpdateLandingDip(float deltaTime);
    };
} // namespace ModularCharacterController
