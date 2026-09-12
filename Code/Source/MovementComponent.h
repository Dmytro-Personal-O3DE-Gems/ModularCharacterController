#pragma once

#include <AzCore/Component/Component.h>
#include <ModularCharacterController/MovementInterface.h>

#include <AzCore/Component/TickBus.h>

#include <PhysX/CharacterGameplayBus.h>

// Input
#include <StartingPointInput/InputEventNotificationBus.h>

#include <AzFramework/Physics/CharacterBus.h>
#include <PhysX/CharacterControllerBus.h>

// Math
#include <AzCore/Math/Vector3.h>            // AZ::Vector3
#include <AzCore/Math/Transform.h>          // AZ::Transform
#include <AzCore/Component/TransformBus.h>  // AZ::TransformBus

#include <AzCore/Math/Crc.h>
#include <AzCore/std/containers/fixed_vector.h>

namespace ModularCharacterController
{
    class MovementComponent
        : public AZ::Component
        , public MovementRequestBus::Handler
        , protected AZ::TickBus::Handler
        , protected Physics::CharacterNotificationBus::Handler
        , protected StartingPointInput::InputEventNotificationBus::MultiHandler
    {
    public:
        AZ_COMPONENT_DECL(MovementComponent);

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

    protected:
        void Activate() override;
        void Deactivate() override;
        virtual void OnTick(float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time);

        void OnCharacterActivated([[maybe_unused]] const AZ::EntityId& entityId) override;

        void OnPressed(float value) override;
        void OnHeld(float value) override;
        void OnReleased([[maybe_unused]] float value) override;

    private:
        // Input variables
        AZ::Vector3 m_currentVelocity = AZ::Vector3::CreateZero();
        float m_moveForward = 0.0f;
        float m_moveRight = 0.0f;
        float m_fWalkSpeed = 3.5f;

   
        // Per-channel speed multipliers
        struct SpeedChannelEntry
        {
            AZ::Crc32 m_channel;
            float     m_scale = 1.0f;
        };

        // A flat list rather than an unordered_map: with 2-5 entries a linear scan over
        // contiguous memory beats a hash lookup, and nothing allocates during a frame.
        static constexpr size_t MaxSpeedChannels = 8;
        AZStd::fixed_vector<SpeedChannelEntry, MaxSpeedChannels> m_speedChannels;

        void  SetSpeedScale(AZ::Crc32 channel, float scale) override;
        float GetSpeedScale(AZ::Crc32 channel) const override;
        float GetTotalSpeedScale() const override;



        float m_fStandingCapsuleHeight = 0.0f;
        float m_fCurrentCapsuleHeight = 0.0f;
        float m_fCapsuleRadius = 0.0f;



        // Backward speed multiplier for when the character is moving backward
        bool isBackwardSpeedMultiplierEnabled = true;
        float m_fBackwardSpeedMultiplier = 0.5f;
        bool isBackwardSpeedMultiplierReadOnly() const { return !isBackwardSpeedMultiplierEnabled; }



        // Acceleration for the character's movement
        bool isAccelerationEnabled = true;
        float m_fAcceleration = 10.0f;
        bool isAccelerationReadOnly() const { return !isAccelerationEnabled; }

        // Fraction of the ground acceleration that still applies while airborne.
        // 1 = full arcade control, 0 = pure inertia. Meaningless without acceleration, because
        // then the velocity snaps to the target and there is no rate left to scale.
        float m_fAirControlFactor = 1.0f;
        bool isAirControlReadOnly() const { return !isAccelerationEnabled; }

        // Input methods
        AZ::Vector3 CalculateLocalMoveDirection() const;
        AZ::Vector3 CalculateWorldMoveDirection() const;
        AZ::Vector3 CalculateAcceleration(float deltaTime, const AZ::Vector3& targetVelocity, float rate) const;

        void ApplyMovement(const AZ::Vector3 worldVelocity);



        // Interface methods
        float GetForwardInput() const override { return m_moveForward; }

        float GetStandingCapsuleHeight() const override { return m_fStandingCapsuleHeight;  }

        float GetCapsuleHeight() const override { return m_fCurrentCapsuleHeight; }
        float GetCapsuleRadius() const override { return m_fCapsuleRadius; }

        void SetCapsuleHeight(float height) override;
        void SetCapsuleRadius(float radius) override;

        bool IsGrounded() override;

        // Input events
        inline static const StartingPointInput::InputEventNotificationId ForwardEventId{ "Forward" };
        inline static const StartingPointInput::InputEventNotificationId BackEventId{ "Back" };
        inline static const StartingPointInput::InputEventNotificationId LeftEventId{ "Left" };
        inline static const StartingPointInput::InputEventNotificationId RightEventId{ "Right" };
    };
} // namespace ModularCharacterController
