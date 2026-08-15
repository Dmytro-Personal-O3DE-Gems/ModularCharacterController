
#pragma once

namespace ModularCharacterController
{
    // System Component TypeIds
    inline constexpr const char* ModularCharacterControllerSystemComponentTypeId = "{AC722251-650C-46C9-A353-7BA19D3CA758}";
    inline constexpr const char* ModularCharacterControllerEditorSystemComponentTypeId = "{87899100-FDB1-4E2A-8D44-5029C3C5C03F}";

    // Module derived classes TypeIds
    inline constexpr const char* ModularCharacterControllerModuleInterfaceTypeId = "{DE420B6B-3747-4066-ACEA-9F1AC7480BF1}";
    inline constexpr const char* ModularCharacterControllerModuleTypeId = "{B14B6CDB-5709-481F-B50A-F4B7116D277E}";
    // The Editor Module by default is mutually exclusive with the Client Module
    // so they use the Same TypeId
    inline constexpr const char* ModularCharacterControllerEditorModuleTypeId = ModularCharacterControllerModuleTypeId;

    // Interface TypeIds
    inline constexpr const char* ModularCharacterControllerRequestsTypeId = "{51EB6B10-70E2-444E-84F9-803CE44A5007}";
} // namespace ModularCharacterController
