#include "CrouchComponent.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/std/algorithm.h>          // AZStd::max
#include <AzCore/Component/TransformBus.h>

namespace ModularCharacterController
{
    AZ_COMPONENT_IMPL(CrouchComponent, "CrouchComponent", "{D2C426CA-363B-4184-BC46-6F95A93143EF}");

    void CrouchComponent::Activate()
    {
        CrouchRequestBus::Handler::BusConnect(GetEntityId());
        StartingPointInput::InputEventNotificationBus::MultiHandler::BusConnect(CrouchEventId);
    }

    void CrouchComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        CrouchRequestBus::Handler::BusDisconnect();
        StartingPointInput::InputEventNotificationBus::MultiHandler::BusDisconnect();
    }

    void CrouchComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CrouchComponent, AZ::Component>()
                ->Version(1)
                ->Field("CrouchHeight", &CrouchComponent::m_fCrouchHeight)
                ->Field("StandUpSafetyMargin", &CrouchComponent::m_fStandUpSafetyMargin)
                ->Field("ToggleMode", &CrouchComponent::m_bToggleMode)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<CrouchComponent>("CrouchComponent", "[Description of functionality provided by this component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "ModularCharacterController")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))

                    ->DataElement(AZ::Edit::UIHandlers::Default, &CrouchComponent::m_fCrouchHeight, "Crouch Height", "Configure crouch height.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &CrouchComponent::m_fStandUpSafetyMargin, "StandUp Safety Margin", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &CrouchComponent::m_bToggleMode, "Toggle Mode", "Hold or press to crouch")
                    ;
            }
        }

        if (AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<CrouchComponent>("Crouch Component Group")
                ->Attribute(AZ::Script::Attributes::Category, "ModularCharacterController Gem Group")
                ;
        }
    }

    void CrouchComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("CrouchComponentService"));
    }

    void CrouchComponent::GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("CrouchComponentService"));
    }

    void CrouchComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("MovementComponentService"));
        required.push_back(AZ_CRC_CE("InputConfigurationService"));
    }

    void CrouchComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }



    void CrouchComponent::OnPressed([[maybe_unused]] float value)
    {
        if (m_bToggleMode)
        {
            m_bIsCrouching ? TryStandUp() : EnterCrouch();
        }
        else
        {
            EnterCrouch();
        }
        AZ_Printf("CrouchComponent", "OnPressed, toggle=%d, isCrouching=%d", m_bToggleMode, m_bIsCrouching);
    }

    void CrouchComponent::OnHeld([[maybe_unused]] float value)
    {
    }

    void CrouchComponent::OnReleased([[maybe_unused]] float value)
    {
        if (m_bToggleMode) return;

        TryStandUp();
        AZ_Printf("CrouchComponent", "OnReleased, toggle=%d, isCrouching=%d", m_bToggleMode, m_bIsCrouching);
    }



    void CrouchComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        TryStandUp();
    }



    void CrouchComponent::EnterCrouch()
    {
        RefreshCapsuleDimensions();

        const float hemisphereCapCount = 2.0f;              // a capsule has two hemispherical end caps, each adds one radius to total height
        const float degenerateHeightSafetyMargin = 0.001f;  // small buffer above the exact degenerate threshold to dodge float rounding and zero-length cylinder edge cases

        const float minValidHeight = hemisphereCapCount * m_fInitialCapsuleRadius + degenerateHeightSafetyMargin;
        m_fAppliedCrouchHeight = AZStd::max(m_fCrouchHeight, minValidHeight);

        MovementRequestBus::Event(GetEntityId(), &MovementRequests::SetCapsuleHeight, m_fAppliedCrouchHeight);

        m_bIsCrouching = true;
        m_bWantsToStand = false;
        AZ::TickBus::Handler::BusDisconnect();
    }

    void CrouchComponent::TryStandUp()
    {
        if (!CanStandUp())
        {
            AZ::TickBus::Handler::BusConnect();
            m_bWantsToStand = true;
            return;
        }

        MovementRequestBus::Event(GetEntityId(), &MovementRequests::SetCapsuleHeight, m_fInitialCapsuleHeight);
        m_bIsCrouching = false;
        m_bWantsToStand = false;
        AZ::TickBus::Handler::BusDisconnect();
    }

    bool CrouchComponent::CanStandUp() const
    {
        auto* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get();
        if (!sceneInterface) { return true; }

        const AzPhysics::SceneHandle sceneHandle = sceneInterface->GetSceneHandle(AzPhysics::DefaultPhysicsSceneName);

        AZ::Transform worldTransform = AZ::Transform::CreateIdentity();
        AZ::TransformBus::EventResult(worldTransform, GetEntityId(), &AZ::TransformBus::Events::GetWorldTM);

        const AZ::Vector3 castStart = worldTransform.GetTranslation()
            + AZ::Vector3(0.0f, 0.0f, m_fAppliedCrouchHeight - m_fInitialCapsuleRadius);

        const float castDistance = (m_fInitialCapsuleHeight - m_fAppliedCrouchHeight) + m_fStandUpSafetyMargin;

        AzPhysics::ShapeCastRequest request = AzPhysics::ShapeCastRequestHelpers::CreateSphereCastRequest(
            m_fInitialCapsuleRadius,
            AZ::Transform::CreateTranslation(castStart),
            AZ::Vector3(0.0f, 0.0f, 1.0f),
            castDistance,
            AzPhysics::SceneQuery::QueryType::StaticAndDynamic,
            AzPhysics::CollisionGroup::All,
            nullptr);

        request.m_reportMultipleHits = true;

        const AzPhysics::SceneQueryHits hits = sceneInterface->QueryScene(sceneHandle, &request);

        const AZ::EntityId selfEntityId = GetEntityId();
        for (const AzPhysics::SceneQueryHit& hit : hits.m_hits)
        {
            if (hit.m_entityId != selfEntityId)
            {
                return false;
            }
        }
        return true;
    }

    void CrouchComponent::RefreshCapsuleDimensions()
    {
        MovementRequestBus::EventResult(
            m_fInitialCapsuleHeight, GetEntityId(), &MovementRequests::GetStandingCapsuleHeight);
        MovementRequestBus::EventResult(
            m_fInitialCapsuleRadius, GetEntityId(), &MovementRequests::GetCapsuleRadius);
    }
} // namespace ModularCharacterController
