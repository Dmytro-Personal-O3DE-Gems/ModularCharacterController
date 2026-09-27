
#include "ViewOffsetComponent.h"

#include <AzCore/std/algorithm.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace ModularCharacterController
{
    AZ_COMPONENT_IMPL(ViewOffsetComponent, "ViewOffsetComponent", "{7834680F-7193-4AAF-B5FC-60FC8271E230}");

    void ViewOffsetComponent::Activate()
    {
        ViewOffsetRequestBus::Handler::BusConnect(GetEntityId());
        AZ::TickBus::Handler::BusConnect();
    }

    void ViewOffsetComponent::Deactivate()
    {
        ViewOffsetRequestBus::Handler::BusDisconnect(GetEntityId());
        AZ::TickBus::Handler::BusDisconnect();
    }

    void ViewOffsetComponent::OnTick(float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        float standingHeight = 0.0f;
        float currentHeight = 0.0f;

        MovementRequestBus::EventResult(standingHeight, GetEntityId(), &MovementRequests::GetStandingCapsuleHeight);
        MovementRequestBus::EventResult(currentHeight, GetEntityId(), &MovementRequests::GetCapsuleHeight);

        // The eye keeps a constant distance from the top of the capsule, and Resize()
        // holds the base in place - so the drop equals the height lost, whoever caused it.
        // Both heights read 0 until OnCharacterActivated fills them in Movement, and 0 - 0
        // is a harmless zero offset.
        const float deltaZ = currentHeight - standingHeight;

		m_fCrouchOffsetZ += (deltaZ - m_fCrouchOffsetZ) * AZStd::clamp(m_fCrouchSmoothingRate * deltaTime, 0.0f, 1.0f);

        m_vTotalOffset = AZ::Vector3(0.0f, 0.0f, m_fCrouchOffsetZ);
    }

    void ViewOffsetComponent::SetOffset([[maybe_unused]] AZ::Crc32 channel, [[maybe_unused]] const AZ::Vector3& offset)
    {
    }

    void ViewOffsetComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<ViewOffsetComponent, AZ::Component>()
                ->Version(1)
                ->Field("CrouchSmoothingRate", &ViewOffsetComponent::m_fCrouchSmoothingRate)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<ViewOffsetComponent>("View Offset",
                    "Owns where the eye sits relative to its authored position. Reports a pure "
                    "offset and never touches the camera itself - the view component adds it to "
                    "the base, which is what keeps first and third person sharing one answer. "
                    "Built-in effects are computed here, each with its own smoothing; outside "
                    "systems such as weapon recoil or an explosion contribute through SetOffset "
                    "on named channels, and everything is summed once per tick.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Modular Character Controller/View")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Crouch")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &ViewOffsetComponent::m_fCrouchSmoothingRate,
                        "Crouch Smoothing Rate",
                        "How quickly the eye follows the capsule when its height changes. The value "
                        "is a rate, not a duration: the eye covers about 63 percent of the remaining "
                        "distance every 1/rate seconds, so 10 settles in roughly a third of a second "
                        "and 20 reads as almost instant. Below 5 the character feels like it is "
                        "moving through water. The capsule itself resizes in one step - only the view "
                        "is smoothed, so the collision shape never lags behind what the player can "
                        "walk under.")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.0f)
                    ;
            }
        }

        if (AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<ViewOffsetComponent>("ViewOffset")
                ->Attribute(AZ::Script::Attributes::Category, "ModularCharacterController Gem Group")
                ;
        }
    }

    void ViewOffsetComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("ViewOffsetComponentService"));
    }

    void ViewOffsetComponent::GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
		incompatible.push_back(AZ_CRC_CE("ViewOffsetComponentService"));
    }

    void ViewOffsetComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void ViewOffsetComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }
} // namespace ModularCharacterController
