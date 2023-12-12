#pragma once
#include"GameObject.h"
#include "Constraint.h"
#include"Vector3.h"
#include"Quaternion.h"

namespace NCL {
	namespace CSC8503 {
		class GameObject;
		class HingeConstraint :public Constraint {
		public:
			HingeConstraint(GameObject* a, GameObject* b);
			~HingeConstraint();
			void UpdateConstraint(float dt)override;
		protected:
			GameObject* objectA;
			GameObject* objectB;
		};
	}
}