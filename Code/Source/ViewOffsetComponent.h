
#pragma once

#include <AzCore/Component/Component.h>
#include <ModularCharacterController/ViewOffsetInterface.h>
#include <ModularCharacterController/MovementInterface.h>

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
        // Computed once per tick, not inside the getter: smoothing will step with
        // deltaTime, and that step must happen once a frame, not once per reader.
        AZ::Vector3 m_vTotalOffset = AZ::Vector3::CreateZero();
        
		float m_fCrouchOffsetZ = 0.0f; // The current offset from the standing height, smoothed over time.
		float m_fCrouchSmoothingRate = 10.0f; // How fast the view catches up to the crouch height change.
    };
} // namespace ModularCharacterController
