
#pragma once

#include <AzCore/Component/Component.h>
#include <ModularCharacterController/GroundTrackerInterface.h>

#include <AzCore/Component/TickBus.h>
#include <PhysX/CharacterGameplayBus.h>

namespace ModularCharacterController
{
    class GroundTrackerComponent
        : public AZ::Component
        , public GroundTrackerRequestBus::Handler
        , protected AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT_DECL(GroundTrackerComponent);

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

		bool GetIsGrounded() override { return m_isGrounded; }

    protected:
        void Activate() override;
        void Deactivate() override;

        void OnTick(float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time) override;

	private:
        bool  m_isGrounded = true;              // engine's own default when no character is present
        float m_fTimeSinceGroundLost = 0.0f;    // grows while the raw signal says "airborne"
        float m_fGroundLossGraceTime = 0.1f;    // reflected, editable

        //! Snapshot of the previous airborne frame. Exists because the engine destroys the real
        //! value the instant it finds ground - see the comment in OnTick.
        float m_fLastAirborneImpactSpeed = 0.0f;

    };
} // namespace ModularCharacterController
