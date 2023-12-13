#include "OrientationConstraint.h"
#include "GameObject.h"
#include "PhysicsObject.h"
using namespace NCL;
using namespace Maths;
using namespace CSC8503;

OrientationConstraint::OrientationConstraint(GameObject* a, GameObject* b)
{
	objectA = a;
	objectB = b;
}

OrientationConstraint::~OrientationConstraint()
{

}

void OrientationConstraint::UpdateConstraint(float dt) {
    if (objectA == nullptr || objectB == nullptr) {
        return; 
    }
    Vector3 relativePos = objectB->GetTransform().GetPosition() - objectA->GetTransform().GetPosition();
    relativePos.y = 0;
    Vector3 dir = relativePos.Normalised();
    objectA->GetTransform().SetOrientation(Quaternion::EulerAnglesToQuaternion(0, Maths::RadiansToDegrees(atan2f(dir.z, dir.x)), 0));
}