
#include "ModularCharacterControllerModuleInterface.h"
#include <AzCore/Memory/Memory.h>

#include <ModularCharacterController/ModularCharacterControllerTypeIds.h>

#include <Clients/ModularCharacterControllerSystemComponent.h>

#include "MovementComponent.h"
#include "SprintComponent.h"

namespace ModularCharacterController
{
    AZ_TYPE_INFO_WITH_NAME_IMPL(ModularCharacterControllerModuleInterface,
        "ModularCharacterControllerModuleInterface", ModularCharacterControllerModuleInterfaceTypeId);
    AZ_RTTI_NO_TYPE_INFO_IMPL(ModularCharacterControllerModuleInterface, AZ::Module);
    AZ_CLASS_ALLOCATOR_IMPL(ModularCharacterControllerModuleInterface, AZ::SystemAllocator);

    ModularCharacterControllerModuleInterface::ModularCharacterControllerModuleInterface()
    {
        // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
        // Add ALL components descriptors associated with this gem to m_descriptors.
        // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
        // This happens through the [MyComponent]::Reflect() function.
        m_descriptors.insert(m_descriptors.end(), {
            ModularCharacterControllerSystemComponent::CreateDescriptor(),
            MovementComponent::CreateDescriptor(),
            SprintComponent::CreateDescriptor(),
            });
    }

    AZ::ComponentTypeList ModularCharacterControllerModuleInterface::GetRequiredSystemComponents() const
    {
        return AZ::ComponentTypeList{
            azrtti_typeid<ModularCharacterControllerSystemComponent>(),
        };
    }
} // namespace ModularCharacterController
