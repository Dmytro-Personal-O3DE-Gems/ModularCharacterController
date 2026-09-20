
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

        Camera::CameraRequestBus::Event(m_cameraEntityId, &Camera::CameraRequestBus::Events::MakeActiveView);

        if (m_cameraEntityId.IsValid())
        {
            AZ::EntityBus::Handler::BusConnect(m_cameraEntityId);
        } 
        else
        {
			AZ_Warning("FirstPersonCameraComponent", false, "Camera entity is not valid. Please set a camera entity in the component properties.");
        }

    }

    void FirstPersonCameraComponent::Deactivate()
    {
        FirstPersonCameraRequestBus::Handler::BusDisconnect(GetEntityId());
        AZ::EntityBus::Handler::BusDisconnect();
    }

    void FirstPersonCameraComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time)
    {
    }

    int FirstPersonCameraComponent::GetTickOrder()
    {
        return AZ::ComponentTickBus::TICK_GAME;
    }

    void FirstPersonCameraComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<FirstPersonCameraComponent, AZ::Component>()
                ->Version(1)
				->Field("CameraEntity", &FirstPersonCameraComponent::m_cameraEntityId)
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
		incompatible.push_back(AZ_CRC_CE("FirstPersonCameraComponentService"));
    }

    void FirstPersonCameraComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
		required.push_back(AZ_CRC_CE("CameraService"));
		required.push_back(AZ_CRC_CE("ViewAnglesComponentService"));
    }

    void FirstPersonCameraComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }
} // namespace ModularCharacterController
