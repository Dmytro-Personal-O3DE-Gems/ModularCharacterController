#pragma once

#include <AzCore/Component/ComponentBus.h>

namespace ModularCharacterController
{
    //! Implemented by whichever view component is currently the active view.
    //!
    //! Addressed by the character's EntityId, not the camera's: every view component lives on
    //! the character entity, so they all share one address. That is deliberate - only the one
    //! whose camera is the active view stays connected, which keeps the handler count at
    //! exactly one and makes EventResult unambiguous without a separate "am I active" flag.
    //!
    //! Nothing outside a view component and ViewAngles may read from this bus. Keeping it to
    //! those two is what makes the answer type cheap to change later: today a bool is enough,
    //! but soft following, follow-only-while-moving and follow-only-while-aiming all need
    //! something richer, and that swap stays a two-file edit for exactly as long as the bus
    //! stays private to this pair.
    class ViewModeRequests
        : public AZ::ComponentBus
    {
    public:
        AZ_RTTI(ModularCharacterController::ViewModeRequests, "{378E17EA-0E75-4765-B3CE-C805CFC47A0F}");

        //! Exactly one view component may be connected at a time - the active one. The default
        //! policy would allow several and silently return whichever answered last, which is a
        //! drifting bug. Single turns that into an assert at the moment the second one connects.
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;

        //! Whether the body should be rotated to match the look yaw this frame.
        //! True for a first person view, and for a third person view that turns the character
        //! with the camera. False for an orbiting view, where the camera circles a body that
        //! stays where it is.
        virtual bool GetBodyFollowsYaw() const = 0
    };

    using ViewModeRequestBus = AZ::EBus<ViewModeRequests>;

} // namespace ModularCharacterController