
#include <ModularCharacterController/ModularCharacterControllerTypeIds.h>
#include <ModularCharacterControllerModuleInterface.h>
#include "ModularCharacterControllerEditorSystemComponent.h"

namespace ModularCharacterController
{
    class ModularCharacterControllerEditorModule
        : public ModularCharacterControllerModuleInterface
    {
    public:
        AZ_RTTI(ModularCharacterControllerEditorModule, ModularCharacterControllerEditorModuleTypeId, ModularCharacterControllerModuleInterface);
        AZ_CLASS_ALLOCATOR(ModularCharacterControllerEditorModule, AZ::SystemAllocator);

        ModularCharacterControllerEditorModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
            // This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(m_descriptors.end(), {
                ModularCharacterControllerEditorSystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList {
                azrtti_typeid<ModularCharacterControllerEditorSystemComponent>(),
            };
        }
    };
}// namespace ModularCharacterController

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME, _Editor), ModularCharacterController::ModularCharacterControllerEditorModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_ModularCharacterController_Editor, ModularCharacterController::ModularCharacterControllerEditorModule)
#endif
