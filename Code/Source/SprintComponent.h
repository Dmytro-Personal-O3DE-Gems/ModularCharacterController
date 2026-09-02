
#pragma once

#include <AzCore/Component/Component.h>
#include <ModularCharacterController/SprintInterface.h>

#include <ModularCharacterController/MovementInterface.h>
#include <ModularCharacterController/CrouchInterface.h>

#include <AzCore/Component/TickBus.h>
#include <StartingPointInput/InputEventNotificationBus.h>

namespace ModularCharacterController
{
    /*
    * TODO: Register this component in your Gem's AZ::Module interface by inserting the following into the list of m_descriptors:
    *       SprintComponent::CreateDescriptor(),
    */

    class SprintComponent
        : public AZ::Component
        , public SprintRequestBus::Handler
        , protected AZ::TickBus::Handler
        , protected StartingPointInput::InputEventNotificationBus::MultiHandler
    {
    public:
        AZ_COMPONENT_DECL(SprintComponent);

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

    protected:
        void Activate() override;
        void Deactivate() override;
        void OnTick([[maybe_unused]]float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time) override;

        void OnPressed(float value) override;
        void OnHeld(float value) override;
        void OnReleased(float value) override;

    private:
        float m_sprintMultiplier = 2.0f;

        bool m_bCanSprintWhileCrouching = false;

        bool m_canSprintBackwards = false;
        bool isSprintBackwardsReadOnly() const { return !m_canSprintBackwards; }

        inline static const StartingPointInput::InputEventNotificationId SprintEventId{ "Sprint" };
    };
} // namespace ModularCharacterController
