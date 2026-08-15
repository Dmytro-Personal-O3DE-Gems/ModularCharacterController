
#include "ModularCharacterControllerSystemComponent.h"

#include <ModularCharacterController/ModularCharacterControllerTypeIds.h>

#include <AzCore/Serialization/SerializeContext.h>

namespace ModularCharacterController
{
    AZ_COMPONENT_IMPL(ModularCharacterControllerSystemComponent, "ModularCharacterControllerSystemComponent",
        ModularCharacterControllerSystemComponentTypeId);

    void ModularCharacterControllerSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<ModularCharacterControllerSystemComponent, AZ::Component>()
                ->Version(0)
                ;
        }
    }

    void ModularCharacterControllerSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("ModularCharacterControllerService"));
    }

    void ModularCharacterControllerSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("ModularCharacterControllerService"));
    }

    void ModularCharacterControllerSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void ModularCharacterControllerSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    ModularCharacterControllerSystemComponent::ModularCharacterControllerSystemComponent()
    {
        if (ModularCharacterControllerInterface::Get() == nullptr)
        {
            ModularCharacterControllerInterface::Register(this);
        }
    }

    ModularCharacterControllerSystemComponent::~ModularCharacterControllerSystemComponent()
    {
        if (ModularCharacterControllerInterface::Get() == this)
        {
            ModularCharacterControllerInterface::Unregister(this);
        }
    }

    void ModularCharacterControllerSystemComponent::Init()
    {
    }

    void ModularCharacterControllerSystemComponent::Activate()
    {
        ModularCharacterControllerRequestBus::Handler::BusConnect();
        AZ::TickBus::Handler::BusConnect();
    }

    void ModularCharacterControllerSystemComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        ModularCharacterControllerRequestBus::Handler::BusDisconnect();
    }

    void ModularCharacterControllerSystemComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
    }

} // namespace ModularCharacterController
