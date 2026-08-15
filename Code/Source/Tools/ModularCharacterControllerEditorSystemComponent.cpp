
#include <AzCore/Serialization/SerializeContext.h>
#include "ModularCharacterControllerEditorSystemComponent.h"

#include <ModularCharacterController/ModularCharacterControllerTypeIds.h>

namespace ModularCharacterController
{
    AZ_COMPONENT_IMPL(ModularCharacterControllerEditorSystemComponent, "ModularCharacterControllerEditorSystemComponent",
        ModularCharacterControllerEditorSystemComponentTypeId, BaseSystemComponent);

    void ModularCharacterControllerEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<ModularCharacterControllerEditorSystemComponent, ModularCharacterControllerSystemComponent>()
                ->Version(0);
        }
    }

    ModularCharacterControllerEditorSystemComponent::ModularCharacterControllerEditorSystemComponent() = default;

    ModularCharacterControllerEditorSystemComponent::~ModularCharacterControllerEditorSystemComponent() = default;

    void ModularCharacterControllerEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        BaseSystemComponent::GetProvidedServices(provided);
        provided.push_back(AZ_CRC_CE("ModularCharacterControllerEditorService"));
    }

    void ModularCharacterControllerEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        BaseSystemComponent::GetIncompatibleServices(incompatible);
        incompatible.push_back(AZ_CRC_CE("ModularCharacterControllerEditorService"));
    }

    void ModularCharacterControllerEditorSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        BaseSystemComponent::GetRequiredServices(required);
    }

    void ModularCharacterControllerEditorSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        BaseSystemComponent::GetDependentServices(dependent);
    }

    void ModularCharacterControllerEditorSystemComponent::Activate()
    {
        ModularCharacterControllerSystemComponent::Activate();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
    }

    void ModularCharacterControllerEditorSystemComponent::Deactivate()
    {
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        ModularCharacterControllerSystemComponent::Deactivate();
    }

} // namespace ModularCharacterController
