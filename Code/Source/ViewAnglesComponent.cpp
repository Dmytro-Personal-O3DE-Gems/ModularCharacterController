
#include "ViewAnglesComponent.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace ModularCharacterController
{
    AZ_COMPONENT_IMPL(ViewAnglesComponent, "ViewAnglesComponent", "{586534C0-8C71-4F9A-A8CF-7EF38C0C3E27}");

    void ViewAnglesComponent::Activate()
    {
        ViewAnglesRequestBus::Handler::BusConnect(GetEntityId());

        StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(LookMouseXEventId);
        StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(LookMouseYEventId);
        StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(LookStickXEventId);
        StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(LookStickYEventId);
    }

    void ViewAnglesComponent::Deactivate()
    {
        ViewAnglesRequestBus::Handler::BusDisconnect(GetEntityId());
        StartingPointInput::InputEventNotificationBus::MultiHandler::BusDisconnect();
    }

    void ViewAnglesComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time)
    {
    }

    int ViewAnglesComponent::GetTickOrder()
    {
        return 0;
    }

    void ViewAnglesComponent::OnPressed(float value)
    {
    }

    void ViewAnglesComponent::OnHeld(float value)
    {
    }

    void ViewAnglesComponent::OnReleased(float value)
    {
    }

    void ViewAnglesComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<ViewAnglesComponent, AZ::Component>()
                ->Version(1)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<ViewAnglesComponent>("ViewAnglesComponent", "[Description of functionality provided by this component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "ComponentCategory")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ;
            }
        }

        if (AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<ViewAnglesComponent>("ViewAngles Component Group")
                ->Attribute(AZ::Script::Attributes::Category, "ModularCharacterController Gem Group")
                ;
        }
    }

    void ViewAnglesComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("ViewAnglesComponentService"));
    }

    void ViewAnglesComponent::GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
    }

    void ViewAnglesComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void ViewAnglesComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }
} // namespace ModularCharacterController
