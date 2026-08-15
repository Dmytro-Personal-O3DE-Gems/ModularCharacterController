
#include <ModularCharacterController/ModularCharacterControllerTypeIds.h>
#include <ModularCharacterControllerModuleInterface.h>
#include "ModularCharacterControllerSystemComponent.h"

namespace ModularCharacterController
{
    class ModularCharacterControllerModule
        : public ModularCharacterControllerModuleInterface
    {
    public:
        AZ_RTTI(ModularCharacterControllerModule, ModularCharacterControllerModuleTypeId, ModularCharacterControllerModuleInterface);
        AZ_CLASS_ALLOCATOR(ModularCharacterControllerModule, AZ::SystemAllocator);
    };
}// namespace ModularCharacterController

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME), ModularCharacterController::ModularCharacterControllerModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_ModularCharacterController, ModularCharacterController::ModularCharacterControllerModule)
#endif
