
#include "MovementComponent.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace ModularCharacterController
{
    AZ_COMPONENT_IMPL(MovementComponent, "MovementComponent", "{91245471-2DC6-43F2-AC7E-EC60DA592DCD}");

    void MovementComponent::Activate()
    {
        MovementRequestBus::Handler::BusConnect(GetEntityId());
        AZ::TickBus::Handler::BusConnect();

        Physics::CharacterNotificationBus::Handler::BusConnect(GetEntityId());

        StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(
            StartingPointInput::InputEventNotificationId("Forward"));
        StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(
            StartingPointInput::InputEventNotificationId("Back"));
        StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(
            StartingPointInput::InputEventNotificationId("Left"));
        StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(
            StartingPointInput::InputEventNotificationId("Right"));
    }

    void MovementComponent::Deactivate()
    {
        MovementRequestBus::Handler::BusDisconnect(GetEntityId());
        AZ::TickBus::Handler::BusDisconnect();
        Physics::CharacterNotificationBus::Handler::BusDisconnect(GetEntityId());
        StartingPointInput::InputEventNotificationBus::MultiHandler::BusDisconnect();
    }

    void MovementComponent::OnTick(float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        const AZ::Vector3 targetVelocity = CalculateWorldMoveDirection();
        
        m_currentVelocity = 
            isAccelerationEnabled ? CalculateAcceleration(deltaTime, targetVelocity) :
                                    targetVelocity;

        ApplyMovement(m_currentVelocity);
    }

    void MovementComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<MovementComponent, AZ::Component>()
                ->Version(1)
                ->Field("WalkSpeed", &MovementComponent::m_fWalkSpeed)
                ->Field("Acceleration", &MovementComponent::m_fAcceleration)
                ->Field("BackwardSpeedMultiplier", &MovementComponent::m_fBackwardSpeedMultiplier)
                ->Field("EnableAcceleration", &MovementComponent::isAccelerationEnabled)
                ->Field("EnableBackwardSpeedMultiplier", &MovementComponent::isBackwardSpeedMultiplierEnabled)

                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<MovementComponent>("Character Controller/Core", "[Description of functionality provided by this component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Character Controller/Core")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))

                    ->DataElement(AZ::Edit::UIHandlers::Default, &MovementComponent::m_fWalkSpeed, "Walk Speed", "The speed at which the character moves.")
                    
                    ->ClassElement(AZ::Edit::ClassElements::Group, "Extra Movement Settings")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &MovementComponent::isAccelerationEnabled, "Enable Acceleration", "Enable or disable acceleration for the character's movement.")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::AttributesAndValues)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &MovementComponent::m_fAcceleration, "Acceleration", "The acceleration of the character.")
                        ->Attribute(AZ::Edit::Attributes::ReadOnly, &MovementComponent::isAccelerationReadOnly)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &MovementComponent::isBackwardSpeedMultiplierEnabled, "Enable Backward Speed Multiplier", "Enable or disable the backward speed multiplier for the character's movement.")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::AttributesAndValues)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &MovementComponent::m_fBackwardSpeedMultiplier, "Backward Speed Multiplier", "The multiplier for the speed when moving backward.")
                        ->Attribute(AZ::Edit::Attributes::ReadOnly, &MovementComponent::isBackwardSpeedMultiplierReadOnly)
    ;
                    ;
            }
        }

        if (AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<MovementComponent>("Movement Component Group")
                ->Attribute(AZ::Script::Attributes::Category, "ModularCharacterController Gem Group")
                ;
        }
    }

    void MovementComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("MovementComponentService"));
    }

    void MovementComponent::GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("MovementComponentService"));
    }

    void MovementComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        // InputConfigurationService is required for the character's input handling
        required.push_back(AZ_CRC_CE("InputConfigurationService"));
        // PhysicsCharacterControllerService is required for the character's physics-based movement
        required.push_back(AZ_CRC_CE("PhysicsCharacterControllerService"));
        required.push_back(AZ_CRC_CE("PhysicsCharacterGameplayService"));
    }

    void MovementComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    // Inputs Event methods
    void MovementComponent::OnPressed(float value) {
        OnHeld(value);
    }

    void MovementComponent::OnHeld(float value) {
        const auto* currentId = StartingPointInput::InputEventNotificationBus::GetCurrentBusId();

        if (*currentId == ForwardEventId)      { m_moveForward = value; }
        else if (*currentId == BackEventId)    { m_moveForward = -value; }
        else if (*currentId == LeftEventId)    { m_moveRight = -value; }
        else if (*currentId == RightEventId)   { m_moveRight = value; }
    }

    void MovementComponent::OnReleased([[maybe_unused]] float value) {
        const auto* currentId = StartingPointInput::InputEventNotificationBus::GetCurrentBusId();

        if (*currentId == ForwardEventId || *currentId == BackEventId)    { m_moveForward = 0.0f; }
        else if (*currentId == LeftEventId || *currentId == RightEventId) { m_moveRight = 0.0f; }
    }

    // Calculate Move Direction methods
    AZ::Vector3 MovementComponent::CalculateLocalMoveDirection() const {
        AZ::Vector3 direction(m_moveRight, m_moveForward, 0.0f);

        if (!direction.IsZero()) { direction.Normalize(); }

        return direction;
    }

    AZ::Vector3 MovementComponent::CalculateWorldMoveDirection() const {
        AZ::Vector3 localMoveDirection = CalculateLocalMoveDirection();
        AZ::Transform worldTransform = AZ::Transform::CreateIdentity();

        const float effectiveWalkSpeed = m_fWalkSpeed * m_fSpeedMultiplier;

        float forwardSpeed = effectiveWalkSpeed;
        if (isBackwardSpeedMultiplierEnabled && localMoveDirection.GetY() < 0.0f)
            forwardSpeed = effectiveWalkSpeed * m_fBackwardSpeedMultiplier;

        AZ::TransformBus::EventResult(worldTransform, GetEntityId(), &AZ::TransformBus::Events::GetWorldTM);

        // World Directions
        AZ::Vector3 worldForward = worldTransform.GetBasisY();
        AZ::Vector3 worldRight = worldTransform.GetBasisX();

        // Resulting World Move Direction
        AZ::Vector3 worldMoveDirection = (
            worldRight * localMoveDirection.GetX() * effectiveWalkSpeed) +        // Left/Right movement
            (worldForward * localMoveDirection.GetY() * forwardSpeed);      // Forward/Backward movement

        return worldMoveDirection;
    }

    AZ::Vector3 MovementComponent::CalculateAcceleration(float deltaTime, const AZ::Vector3& targetVelocity) const {
        float t = AZStd::clamp(m_fAcceleration * deltaTime, 0.0f, 1.0f);
        return m_currentVelocity.Lerp(targetVelocity, t);
    }

    void MovementComponent::ApplyMovement(const AZ::Vector3 worldVelocity) {
        Physics::CharacterRequestBus::Event(
            GetEntityId(),
            &Physics::CharacterRequestBus::Events::AddVelocityForTick,
            worldVelocity
        );
    }
    // ~Calculate Move Direction methods
    
} // namespace ModularCharacterController
