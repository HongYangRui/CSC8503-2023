#include"HingeConstraint.h"
#include "Vector3.h"
#include "GameObject.h"
#include "PhysicsObject.h"
#include "Debug.h"




using namespace NCL;
using namespace Maths;
using namespace CSC8503;

HingeConstraint::HingeConstraint(GameObject* a, GameObject* b) {
	objectA = a;
	objectB = b;
}

HingeConstraint::~HingeConstraint() {

}

void HingeConstraint::UpdateConstraint(float dt) {
	Vector3 relativePos = objectA->GetTransform().GetPosition() - objectB->GetTransform().GetPosition();
	relativePos.y = 0;
	Vector3 dir = relativePos.Normalised();
	objectA->GetTransform().SetOrientation(Quaternion::EulerAnglesToQuaternion(0, -Maths::RadiansToDegrees(atan2f(dir.z, dir.x)), 0));
	objectB->GetTransform().SetOrientation(Quaternion::EulerAnglesToQuaternion(0, Maths::RadiansToDegrees(atan2f(-dir.z, dir.x)), 0));
}