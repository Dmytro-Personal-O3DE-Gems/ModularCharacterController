
#include "JumpComponent.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace ModularCharacterController
{
    AZ_COMPONENT_IMPL(JumpComponent, "JumpComponent", "{B3252609-1EC1-4961-90C5-81172DDEDB4B}");

    
    void JumpComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<JumpComponent, AZ::Component>()
                ->Version(1)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<JumpComponent>("JumpComponent", "[Description of functionality provided by this component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "ModularCharacterController")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ;
            }
        }
    }


    void JumpComponent::Init()
    {
        // Built once per component lifetime - the callback never changes between activations.
        // The event carries (SceneHandle, gravity); the scene is always ours, so the first
        // parameter is ignored.
        m_onGravityChangedHandler = AzPhysics::SceneEvents::OnSceneGravityChangedEvent::Handler(
            [this]([[maybe_unused]] AzPhysics::SceneHandle sceneHandle, const AZ::Vector3& newGravity)
            {
                m_gravity = newGravity;
            });
    }

    void JumpComponent::Activate()
    {
        JumpRequestBus::Handler::BusConnect(GetEntityId());
        AZ::TickBus::Handler::BusConnect();

        StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(JumpEventId);

        SetupSceneGravity();
    }

    void JumpComponent::Deactivate()
    {
        JumpRequestBus::Handler::BusDisconnect(GetEntityId());
        AZ::TickBus::Handler::BusDisconnect();

        StartingPointInput::InputEventNotificationBus::MultiHandler::BusDisconnect();

        m_onGravityChangedHandler.Disconnect();
    }


    void JumpComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("JumpComponentService"));
    }

    void JumpComponent::GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
    }

    void JumpComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        // InputConfigurationService is required for the character's input handling
        required.push_back(AZ_CRC_CE("InputConfigurationService"));
        // PhysicsCharacterControllerService is required for the character's physics-based movement
        required.push_back(AZ_CRC_CE("PhysicsCharacterControllerService"));
        required.push_back(AZ_CRC_CE("PhysicsCharacterGameplayService"));
    }

    void JumpComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }



    void JumpComponent::OnPressed([[maybe_unused]] float value)
    {
        Physics::CharacterRequestBus::Event(
            GetEntityId(), &Physics::CharacterRequests::AddVelocityForTick, AZ::Vector3(0.0f, 0.0f, m_fJumpSpeed));

        PhysX::CharacterGameplayRequestBus::Event(
            GetEntityId(), &PhysX::CharacterGameplayRequests::SetFallingVelocity, AZ::Vector3(0.0f, 0.0f, m_fJumpSpeed));
    }

    void JumpComponent::OnHeld([[maybe_unused]] float value)
    {
    }

    void JumpComponent::OnReleased([[maybe_unused]] float value)
    {
    }



    void JumpComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        bool isGrounded = false;
        MovementRequestBus::EventResult(isGrounded, GetEntityId(), &MovementRequests::IsGrounded);

        AZ_Printf("JumpComponent", "IsGrounded = %s", isGrounded ? "true" : "false");

        AZ::Vector3 velocityZ = AZ::Vector3::CreateZero();

        PhysX::CharacterGameplayRequestBus::EventResult(velocityZ, GetEntityId(), &PhysX::CharacterGameplayRequests::GetFallingVelocity);

        AZ_Printf("JumpComponent", "FallingVelocity = %.3f", velocityZ.GetZ());

    }

    int JumpComponent::GetTickOrder()
    {
        return AZ::ComponentTickBus::TICK_GAME;
    }

    bool JumpComponent::IsAirborne() const
    {
        return false;
    }



    bool JumpComponent::CanJumpNow() const
    {
        return false;
    }

    bool JumpComponent::ResolveCrouch()
    {
        return false;
    }

    void JumpComponent::DoJump()
    {
    }

    bool JumpComponent::CheckHeadHit() const
    {
        return false;
    }



    void JumpComponent::SetupSceneGravity()
    {
        AzPhysics::SceneHandle sceneHandle = AzPhysics::InvalidSceneHandle;
        Physics::DefaultWorldBus::BroadcastResult(sceneHandle,
            &Physics::DefaultWorldRequests::GetDefaultSceneHandle);

        if (sceneHandle == AzPhysics::InvalidSceneHandle)
        {
            AZ_Error("JumpComponent", false, "No default physics scene available.");
            return;
        }

        if (auto* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get())
        {
            // Current value...
            m_gravity = sceneInterface->GetGravity(sceneHandle);
            // ...and every change from now on.
            sceneInterface->RegisterSceneGravityChangedEvent(sceneHandle, m_onGravityChangedHandler);
        }
    }
    
} // namespace ModularCharacterController
