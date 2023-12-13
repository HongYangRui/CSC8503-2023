#pragma once
#include "GameWorld.h"

namespace NCL {
	namespace CSC8503 {
		class PhysicsSystem	{
		public:
			void SetGooseDoor(bool value) {
				GooseDoor = value;
			}
			bool IsGooseDoor()const {
				return GooseDoor;
			}
			void SetGoosePlayer(bool value) {
				GooseDoor = value;
			}
			bool IsGoosePlayer()const {
				return GoosePlayer;
			}
			PhysicsSystem(GameWorld& g);
			~PhysicsSystem();

			void Clear();

			void Update(float dt);

			void UseGravity(bool state) {
				applyGravity = state;
			}

			void SetGlobalDamping(float d) {
				globalDamping = d;
			}

			void SetGravity(const Vector3& g);
		protected:
			void BasicCollisionDetection();
			void BroadPhase();
			void NarrowPhase();

			void ClearForces();

			void IntegrateAccel(float dt);
			void IntegrateVelocity(float dt);

			void UpdateConstraints(float dt);

			void UpdateCollisionList();
			void UpdateObjectAABBs();

			void ImpulseResolveCollision(GameObject& a , GameObject&b, CollisionDetection::ContactPoint& p) const;

			GameWorld& gameWorld;

			bool	applyGravity;
			Vector3 gravity;
			float	dTOffset;
			float	globalDamping;

			std::set<CollisionDetection::CollisionInfo> allCollisions;
			std::set<CollisionDetection::CollisionInfo> broadphaseCollisions;
			std::vector<CollisionDetection::CollisionInfo> broadphaseCollisionsVec;
			bool useBroadPhase		= true;
			int numCollisionFrames	= 5;
			int fraction = 0;
			int GooseScore = 0;
			int destroynum = 9;
			int TPtime1 = 40;
			int TPtime2 = 40;
			int Key = 0;
			int winorlose = 0;
			bool GooseDoor = false;
			bool GoosePlayer = false;
			
		};
	}
}

