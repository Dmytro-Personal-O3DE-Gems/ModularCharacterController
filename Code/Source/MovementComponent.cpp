
#include "MovementComponent.h"

#include <AzCore/Interface/Interface.h>
#include <AzFramework/Physics/PhysicsSystem.h>

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
        StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(
            StartingPointInput::InputEventNotificationId("Jump"));

        ValidateSolidCollisionGroup();
    }

    void MovementComponent::Deactivate()
    {
        MovementRequestBus::Handler::BusDisconnect(GetEntityId());
        AZ::TickBus::Handler::BusDisconnect();
        Physics::CharacterNotificationBus::Handler::BusDisconnect(GetEntityId());
        StartingPointInput::InputEventNotificationBus::MultiHandler::BusDisconnect();
    }

    AzPhysics::CollisionGroup MovementComponent::GetSolidCollisionGroup() const
    {
        // The id is resolved here and nowhere else. Every lookup in this API - by id or by
        // name - answers CollisionGroup::All when it does not find the preset, so a caller
        // doing its own resolve would quietly widen its query and have no way to tell.
        // A null id lands on the same All, which is also the right answer for a project with
        // nothing to exclude; ValidateSolidCollisionGroup() separates the two cases.
        return AzPhysics::GetCollisionGroupById(m_solidCollisionGroupId);
    }

    void MovementComponent::ValidateSolidCollisionGroup() const
    {
        // An unset field is a valid state of the world, not a misconfiguration: a project with
        // no clip geometry has nothing to exclude. Warning here would fire in every honestly
        // configured project on every activation, and a warning that always fires is one that
        // stops being read before the day it matters.
        if (m_solidCollisionGroupId.m_id.IsNull())
        {
            return;
        }

        auto* physicsSystem = AZ::Interface<AzPhysics::SystemInterface>::Get();
        if (physicsSystem == nullptr)
        {
            return;
        }

        const AzPhysics::SystemConfiguration* configuration = physicsSystem->GetConfiguration();
        if (configuration == nullptr)
        {
            return;
        }

        // An empty name means the id points at a preset that is gone. Renaming a preset keeps
        // its id, so this really does mean deleted - or that the gem was dropped into a project
        // that never had the group. Unlike the empty field above, somebody made a choice here
        // and the choice has since broken, while the queries carry on against everything.
        const AZStd::string groupName =
            configuration->m_collisionConfig.m_collisionGroups.FindGroupNameById(m_solidCollisionGroupId);

        AZ_Warning("MovementComponent", !groupName.empty(),
            "Entity '%s' points at a collision group preset that no longer exists in this "
            "project's PhysX configuration. Checks that ask what is solid for this character "
            "will fall back to colliding with everything. Pick the group again on the Character "
            "Movement component, or clear the field if there is nothing to exclude.",
            GetEntity() != nullptr ? GetEntity()->GetName().c_str() : "<unknown>");
    }

    void MovementComponent::OnTick(float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        const AZ::Vector3 targetVelocity = CalculateWorldMoveDirection();

        // Asked once per tick on purpose: IsGrounded runs a fresh overlap query on every call.
        bool isGrounded = false;

        GroundTrackerRequestBus::EventResult(
			isGrounded, GetEntityId(), &GroundTrackerRequests::GetIsGrounded);

        // Air control scales how fast the current velocity is allowed to chase the target while
        // the character is off the ground. A factor of 1 behaves exactly like the ground, 0
        // leaves the take-off velocity untouched, and the values in between let the player nudge
        // the trajectory without reversing it.
        const float accelerationRate = isGrounded
            ? m_fAcceleration
            : m_fAcceleration * m_fAirControlFactor;

        m_currentVelocity =
            isAccelerationEnabled ? CalculateAcceleration(deltaTime, targetVelocity, accelerationRate) :
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
                ->Field("AirControlFactor", &MovementComponent::m_fAirControlFactor)
                ->Field("SolidCollisionGroup", &MovementComponent::m_solidCollisionGroupId)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<MovementComponent>("Character Movement (Core)",
                    "Core of the character controller. Reads the movement keys, accelerates and brakes, "
                    "and applies the resulting velocity to the PhysX capsule. Also owns the capsule "
                    "dimensions and the per-channel speed multipliers that Sprint and Crouch write into, "
                    "and answers whether the character is standing on the ground. Every other component "
                    "in this gem requires it on the same entity.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Modular Character Controller/Core")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))

                    ->DataElement(AZ::Edit::UIHandlers::Default, &MovementComponent::m_fWalkSpeed, "Walk Speed", "The speed at which the character moves.")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Extra Movement Settings")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &MovementComponent::isAccelerationEnabled, "Enable Acceleration", "Enable or disable acceleration for the character's movement.")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::AttributesAndValues)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &MovementComponent::m_fAcceleration, "Acceleration", "The acceleration of the character.")
                    ->Attribute(AZ::Edit::Attributes::ReadOnly, &MovementComponent::isAccelerationReadOnly)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &MovementComponent::m_fAirControlFactor, "Air Control Factor",
                        "How much of the ground acceleration still applies while the character is "
                        "airborne. 1 gives full arcade control - releasing the key stops the character "
                        "in mid-air. 0 gives pure inertia - the take-off velocity is kept until landing "
                        "and input is ignored. Small values let the player nudge the trajectory without "
                        "reversing it. Has no effect unless Acceleration is enabled, because without it "
                        "the velocity snaps straight to the target.")
                    ->Attribute(AZ::Edit::Attributes::ReadOnly, &MovementComponent::isAirControlReadOnly)
                        ->Attribute(AZ::Edit::Attributes::ReadOnly, &MovementComponent::isAccelerationReadOnly)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &MovementComponent::isBackwardSpeedMultiplierEnabled, "Enable Backward Speed Multiplier", "Enable or disable the backward speed multiplier for the character's movement.")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::AttributesAndValues)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &MovementComponent::m_fBackwardSpeedMultiplier, "Backward Speed Multiplier", "The multiplier for the speed when moving backward.")
                        ->Attribute(AZ::Edit::Attributes::ReadOnly, &MovementComponent::isBackwardSpeedMultiplierReadOnly)

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Collision")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &MovementComponent::m_solidCollisionGroupId,
                        "Solid Collision Group",
                        "Which collision group the character's own checks treat as solid - today the "
                        "clearance check before standing up out of a crouch. Leaving it empty is a "
                        "valid choice and not a mistake: with nothing to exclude, colliding with "
                        "everything is the correct answer, and that is exactly what an empty field "
                        "resolves to. Set it once the level gains geometry the character must walk "
                        "on but must not see from these checks - an invisible ramp over a staircase "
                        "being the usual case. Groups themselves are authored per project in the "
                        "PhysX Configuration window, so a gem cannot ship one: collision layers are "
                        "numbered slots in a fixed table and two gems would claim the same slot.")
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
        required.push_back(AZ_CRC_CE("GroundTrackerComponentService"));
    }

    void MovementComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }
    


    void MovementComponent::OnCharacterActivated([[maybe_unused]] const AZ::EntityId& entityId) {
        PhysX::CharacterControllerRequestBus::EventResult(
            m_fStandingCapsuleHeight, GetEntityId(), &PhysX::CharacterControllerRequests::GetHeight);

        PhysX::CharacterControllerRequestBus::EventResult(
            m_fCapsuleRadius, GetEntityId(), &PhysX::CharacterControllerRequests::GetRadius);

        // Nobody has resized the capsule yet, so the character starts at full height. Without
        // this, GetCapsuleHeight reports zero until the first crouch.
        m_fCurrentCapsuleHeight = m_fStandingCapsuleHeight;
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

        const float effectiveWalkSpeed = m_fWalkSpeed * GetTotalSpeedScale();

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

    AZ::Vector3 MovementComponent::CalculateAcceleration(float deltaTime, const AZ::Vector3& targetVelocity, float rate) const {
        // rate is not an acceleration in m/s^2 - it is the fraction of the remaining gap closed
        // per second. The caller decides what it is, which is where air control comes in.
        float t = AZStd::clamp(rate * deltaTime, 0.0f, 1.0f);
        return m_currentVelocity.Lerp(targetVelocity, t);
    }

    void MovementComponent::ApplyMovement(const AZ::Vector3 worldVelocity) {
        Physics::CharacterRequestBus::Event(
            GetEntityId(),
            &Physics::CharacterRequestBus::Events::AddVelocityForTick,
            worldVelocity
        );
    }



    // Interface methods
    void MovementComponent::SetSpeedScale(AZ::Crc32 channel, float scale)
    {
        if (scale < 0.0f)
        {
            AZ_Warning("MovementComponent", false,
                    "SetSpeedScale: negative scale %.3f on channel 0x%08X was clamped to 0. "
                    "A negative multiplier would reverse the movement direction.",
                scale, static_cast<AZ::u32>(channel));
            scale = 0.0f;
        }

        // Channel already registered - just update its contribution
        for (SpeedChannelEntry& entry : m_speedChannels)
        {
            if (entry.m_channel == channel)
            {
                entry.m_scale = scale;
                return;
            }
        }

        if (scale == 1.0f)
        {
            return;
        }

        if (m_speedChannels.size() == m_speedChannels.capacity())
        {
            AZ_Warning("MovementComponent", false,
                "SetSpeedScale: channel limit reached (%zu), channel 0x%08X ignored.",
                m_speedChannels.capacity(), static_cast<AZ::u32>(channel));
            return;
        }

        m_speedChannels.push_back(SpeedChannelEntry{ channel, scale });
    }

    float MovementComponent::GetSpeedScale(AZ::Crc32 channel) const
    {
        for (const SpeedChannelEntry& entry : m_speedChannels)
        {
            if (entry.m_channel == channel)
            {
                return entry.m_scale;
            }
        }
        return 1.0f; // channel was never registered, so it contributes nothin
    }

    float MovementComponent::GetTotalSpeedScale() const
    {
        float total = 1.0f;
        for (const SpeedChannelEntry& entry : m_speedChannels)
        {
            total *= entry.m_scale;
        }
        return total;
    }


    void MovementComponent::SetCapsuleHeight(float height)
    {
        PhysX::CharacterControllerRequestBus::Event(
            GetEntityId(), &PhysX::CharacterControllerRequests::Resize, height);

        m_fCurrentCapsuleHeight = height;

        float actualHeight = 0.0f;
        PhysX::CharacterControllerRequestBus::EventResult(
            actualHeight, GetEntityId(), &PhysX::CharacterControllerRequests::GetHeight);
        AZ_Printf("MovementComponent", "SetCapsuleHeight requested=%.3f actualAfterResize=%.3f", height, actualHeight);
    }

    void MovementComponent::SetCapsuleRadius(float radius)
    {
        PhysX::CharacterControllerRequestBus::Event(
            GetEntityId(), &PhysX::CharacterControllerRequests::SetRadius, radius);

        m_fCapsuleRadius = radius;
    }
    // ~Calculate Move Direction methods

} // namespace ModularCharacterController
