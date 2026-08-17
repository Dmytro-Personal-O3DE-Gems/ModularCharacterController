
#pragma once

#include <AzCore/Component/Component.h>
#include <ModularCharacterController/CrouchInterface.h>

// Cast
#include <AzFramework/Physics/PhysicsSystem.h>             // AzPhysics::SystemInterface — GetSceneHandle / GetScene
#include <AzFramework/Physics/PhysicsScene.h>               // AzPhysics::Scene — method QueryScene(request)
#include <AzFramework/Physics/Common/PhysicsSceneQueries.h> // RayCastRequest / ShapeCastRequest / OverlapRequest / SceneQueryHits / *RequestHelpers
#include <AzFramework/Physics/Collision/CollisionGroups.h>  // CollisionGroup

namespace ModularCharacterController
{
    class CrouchComponent
        : public AZ::Component
        , public CrouchRequestBus::Handler
    {
    public:
        AZ_COMPONENT_DECL(CrouchComponent);

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

    protected:
        void Activate() override;
        void Deactivate() override;
    };
} // namespace ModularCharacterController
