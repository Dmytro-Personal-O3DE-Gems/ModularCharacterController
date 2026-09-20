
#include "FirstPersonCameraComponent.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace ModularCharacterController
{
    AZ_COMPONENT_IMPL(FirstPersonCameraComponent, "FirstPersonCameraComponent", "{2FF67FCB-9CFD-4802-B629-E7A506B6A085}");

    void FirstPersonCameraComponent::Activate()
    {
        FirstPersonCameraRequestBus::Handler::BusConnect(GetEntityId());

        if (m_cameraEntityId.IsValid())
        {
            AZ::EntityBus::Handler::BusConnect(m_cameraEntityId);
        }

    }

    void FirstPersonCameraComponent::Deactivate()
    {
        FirstPersonCameraRequestBus::Handler::BusDisconnect(GetEntityId());
        AZ::EntityBus::Handler::BusDisconnect();
    }

    void FirstPersonCameraComponent::OnCharacterActivated(const AZ::EntityId& entityId)
    {
    }

    void FirstPersonCameraComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<FirstPersonCameraComponent, AZ::Component>()
                ->Version(1)
				->Field("CameraEntityId", &FirstPersonCameraComponent::m_cameraEntityId)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<FirstPersonCameraComponent>("FirstPersonCameraComponent", "[Description of functionality provided by this component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Modular Character Controller/View")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))

					->DataElement(AZ::Edit::UIHandlers::EntityId, &FirstPersonCameraComponent::m_cameraEntityId, "Camera Entity", "The entity that represents the first-person camera.")
                    ;
            }
        }

        if (AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<FirstPersonCameraComponent>("FirstPersonCamera Component Group")
                ->Attribute(AZ::Script::Attributes::Category, "ModularCharacterController Gem Group")
                ;
        }
    }

    void FirstPersonCameraComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("FirstPersonCameraComponentService"));
    }

    void FirstPersonCameraComponent::GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
    }

    void FirstPersonCameraComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void FirstPersonCameraComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }
} // namespace ModularCharacterController
