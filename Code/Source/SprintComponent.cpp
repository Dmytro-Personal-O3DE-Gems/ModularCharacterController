
#include "SprintComponent.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace ModularCharacterController
{
    AZ_COMPONENT_IMPL(SprintComponent, "SprintComponent", "{B842EBDB-25EC-47D1-B54E-B0ED10B488B5}");

    void SprintComponent::Activate()
    {
        SprintRequestBus::Handler::BusConnect(GetEntityId());

        StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(SprintEventId);
    }

    void SprintComponent::Deactivate()
    {
        SprintRequestBus::Handler::BusDisconnect(GetEntityId());
        AZ::TickBus::Handler::BusDisconnect();
        StartingPointInput::InputEventNotificationBus::MultiHandler::BusDisconnect();
    }

    void SprintComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        float forwardInput = 0.0f;
        MovementRequestBus::EventResult(forwardInput, GetEntityId(), &MovementRequests::GetForwardInput);

        const bool blockedByBackward = (!m_canSprintBackwards && forwardInput < 0.0f);
        const float multiplier = blockedByBackward ? 1.0f : m_sprintMultiplier;

        MovementRequestBus::Event(GetEntityId(), &MovementRequests::SetSpeedMultiplier, multiplier);
    }

    void SprintComponent::Reflect(AZ::ReflectContext *context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext *>(context))
        {
            serializeContext->Class<SprintComponent, AZ::Component>()
                ->Version(1)
                ->Field("SprintMultiplier", &SprintComponent::m_sprintMultiplier)
                ->Field("CanSprintBackwards", &SprintComponent::m_canSprintBackwards);

            if (AZ::EditContext *editContext = serializeContext->GetEditContext())
            {
                editContext->Class<SprintComponent>("SprintComponent", "[Description of functionality provided by this component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "ModularCharacterController")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))

                    ->DataElement(AZ::Edit::UIHandlers::Default, &SprintComponent::m_sprintMultiplier, "Sprint Multiplier", "The multiplier applied to the character's movement speed while sprinting.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SprintComponent::m_canSprintBackwards, "Can Sprint Backwards", "Whether the character can sprint while moving backwards.");
            }
        }

        if (AZ::BehaviorContext *behaviorContext = azrtti_cast<AZ::BehaviorContext *>(context))
        {
            behaviorContext->Class<SprintComponent>("Sprint Component Group")
                ->Attribute(AZ::Script::Attributes::Category, "ModularCharacterController Gem Group");
        }
    }


    
    void SprintComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType &provided)
    {
        provided.push_back(AZ_CRC_CE("SprintComponentService"));
    }

    void SprintComponent::GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType &incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("SprintComponentService"));
    }

    void SprintComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType &required)
    {
        required.push_back(AZ_CRC_CE("MovementComponentService"));
        required.push_back(AZ_CRC_CE("InputConfigurationService"));
    }

    void SprintComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType &dependent)
    {
    }



    void SprintComponent::OnPressed([[maybe_unused]] float value)
    {
        AZ::TickBus::Handler::BusConnect();
    }

    void SprintComponent::OnHeld([[maybe_unused]] float value)
    {

    }

    void SprintComponent::OnReleased([[maybe_unused]] float value)
    {
        AZ::TickBus::Handler::BusDisconnect();
        MovementRequestBus::Event(GetEntityId(), &MovementRequests::SetSpeedMultiplier, 1.0f);
    }
} // namespace ModularCharacterController
