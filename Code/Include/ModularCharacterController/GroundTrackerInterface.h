
#pragma once

#include <AzCore/Component/ComponentBus.h>

namespace ModularCharacterController
{
    class GroundTrackerRequests
        : public AZ::ComponentBus
    {
    public:
        AZ_RTTI(ModularCharacterController::GroundTrackerRequests, "{88738ED2-57B3-4C76-B47D-F605F7AFF8CD}");

        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;

        virtual bool GetIsGrounded() = 0;
    };

    using GroundTrackerRequestBus = AZ::EBus<GroundTrackerRequests>;

    //! What a landing carries. A struct rather than a bare float so the payload can gain
    //! fields later without touching the notification signature or any existing listener.
    struct LandingInfo
    {
        AZ_TYPE_INFO(ModularCharacterController::LandingInfo, "{C386542A-F0C6-4DD5-8AB8-27D9571DE44E}");

        //! Vertical speed measured on the last airborne frame, in m/s. Always >= 0:
        //! it is a magnitude, so no sign convention has to be agreed on.
        float m_impactSpeed = 0.0f;

        //! How long the character was airborne before this landing, in seconds.
        float m_airborneTime = 0.0f;
    };

    //! Sent by the component that owns ground state. Several listeners are expected -
    //! camera dip, fall damage, footstep sound - so the default Multiple handler policy
    //! is deliberate here, unlike on the request bus above.
    class GroundTrackerNotifications
        : public AZ::ComponentBus
    {
    public:
        AZ_RTTI(ModularCharacterController::GroundTrackerNotifications, "{91BEF533-028F-48F8-B0BD-809011376EB2}");

        //! Not pure virtual on purpose: a listener that only cares about future events of
        //! this bus must not be forced to implement this one. Same convention as
        //! Physics::CharacterNotifications in the engine.
        virtual void OnLanded([[maybe_unused]] const LandingInfo& landing) {}
    };

    using GroundTrackerNotificationBus = AZ::EBus<GroundTrackerNotifications>;

} // namespace ModularCharacterController
