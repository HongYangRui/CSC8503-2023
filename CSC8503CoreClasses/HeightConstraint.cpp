#include "HeightConstraint.h"
#include "Vector3.h"
#include "GameObject.h"
#include "Debug.h"

using namespace NCL;
using namespace NCL::Maths;
using namespace CSC8503;

HeightConstraint::HeightConstraint(GameObject* a, float h)
{
	objectA = a;
	height = h;
}

HeightConstraint::~HeightConstraint()
{
}

void HeightConstraint::UpdateConstraint(float dt)
{
	Vector3 position = objectA->GetTransform().GetPosition();
	position.y = height;

	objectA->GetTransform().SetPosition(position);
}