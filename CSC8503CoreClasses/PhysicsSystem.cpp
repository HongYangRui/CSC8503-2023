#include "PhysicsSystem.h"
#include "PhysicsObject.h"
#include "GameObject.h"
#include "CollisionDetection.h"
#include "Quaternion.h"

#include "Constraint.h"

#include "Debug.h"
#include "Window.h"
#include <functional>
#include <PositionConstraint.h>
#include <HingeConstraint.h>
#include <HeightConstraint.h>
using namespace NCL;
using namespace CSC8503;

PhysicsSystem::PhysicsSystem(GameWorld& g) : gameWorld(g)	{
	applyGravity	= true;
	useBroadPhase	= false;	
	dTOffset		= 0.0f;
	globalDamping	= 0.995f;
	SetGravity(Vector3(0.0f, -19.8f, 0.0f));
}

PhysicsSystem::~PhysicsSystem()	{
}

void PhysicsSystem::SetGravity(const Vector3& g) {
	gravity = g;
}

/*

If the 'game' is ever reset, the PhysicsSystem must be
'cleared' to remove any old collisions that might still
be hanging around in the collision list. If your engine
is expanded to allow objects to be removed from the world,
you'll need to iterate through this collisions list to remove
any collisions they are in.

*/
void PhysicsSystem::Clear() {
	allCollisions.clear();
}

/*

This is the core of the physics engine update

*/

bool useSimpleContainer = false;

int constraintIterationCount = 10;

//This is the fixed timestep we'd LIKE to have
const int   idealHZ = 120;
const float idealDT = 1.0f / idealHZ;

/*
This is the fixed update we actually have...
If physics takes too long it starts to kill the framerate, it'll drop the 
iteration count down until the FPS stabilises, even if that ends up
being at a low rate. 
*/
int realHZ		= idealHZ;
float realDT	= idealDT;

void PhysicsSystem::Update(float dt) {	
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::B)) {
		useBroadPhase = !useBroadPhase;
		std::cout << "Setting broadphase to " << useBroadPhase << std::endl;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::N)) {
		useSimpleContainer = !useSimpleContainer;
		std::cout << "Setting broad container to " << useSimpleContainer << std::endl;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::I)) {
		constraintIterationCount--;
		std::cout << "Setting constraint iterations to " << constraintIterationCount << std::endl;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::O)) {
		constraintIterationCount++;
		std::cout << "Setting constraint iterations to " << constraintIterationCount << std::endl;
	}

	dTOffset += dt; //We accumulate time delta here - there might be remainders from previous frame!

	GameTimer t;
	t.GetTimeDeltaSeconds();

	if (useBroadPhase) {
		UpdateObjectAABBs();
	}
	int iteratorCount = 0;
	while(dTOffset > realDT) {
		IntegrateAccel(realDT); //Update accelerations from external forces
		if (useBroadPhase) {
			BroadPhase();
			NarrowPhase();
			if (TPtime1 < 0)TPtime1 = 40;
			if (TPtime2 < 0)TPtime2 = 40;
		}
		else {
			BasicCollisionDetection();
			if (TPtime1 < 0)TPtime1 = 40;
			if (TPtime2 < 0)TPtime2 = 40;
		}
		

		//This is our simple iterative solver - 
		//we just run things multiple times, slowly moving things forward
		//and then rechecking that the constraints have been met		
		float constraintDt = realDT /  (float)constraintIterationCount;
		for (int i = 0; i < constraintIterationCount; ++i) {
			UpdateConstraints(constraintDt);	
		}
		IntegrateVelocity(realDT); //update positions from new velocity changes

		dTOffset -= realDT;
		iteratorCount++;
	}

	ClearForces();	//Once we've finished with the forces, reset them to zero

	UpdateCollisionList(); //Remove any old collisions

	t.Tick();
	float updateTime = t.GetTimeDeltaSeconds();

	//Uh oh, physics is taking too long...
	if (updateTime > realDT) {
		realHZ /= 2;
		realDT *= 2;
		std::cout << "Dropping iteration count due to long physics time...(now " << realHZ << ")\n";
	}
	else if(dt*2 < realDT) { //we have plenty of room to increase iteration count!
		int temp = realHZ;
		realHZ *= 2;
		realDT /= 2;

		if (realHZ > idealHZ) {
			realHZ = idealHZ;
			realDT = idealDT;
		}
		if (temp != realHZ) {
			std::cout << "Raising iteration count due to short physics time...(now " << realHZ << ")\n";
		}
	}
}

/*
Later on we're going to need to keep track of collisions
across multiple frames, so we store them in a set.

The first time they are added, we tell the objects they are colliding.
The frame they are to be removed, we tell them they're no longer colliding.

From this simple mechanism, we we build up gameplay interactions inside the
OnCollisionBegin / OnCollisionEnd functions (removing health when hit by a 
rocket launcher, gaining a point when the player hits the gold coin, and so on).
*/
void PhysicsSystem::UpdateCollisionList() {
	for (std::set<CollisionDetection::CollisionInfo>::iterator i = allCollisions.begin(); i != allCollisions.end(); ) {
		if ((*i).framesLeft == numCollisionFrames) {
			i->a->OnCollisionBegin(i->b);
			i->b->OnCollisionBegin(i->a);
		}

		CollisionDetection::CollisionInfo& in = const_cast<CollisionDetection::CollisionInfo&>(*i);
		in.framesLeft--;

		if ((*i).framesLeft < 0) {
			i->a->OnCollisionEnd(i->b);
			i->b->OnCollisionEnd(i->a);
			i = allCollisions.erase(i);
		}
		else {
			++i;
		}
	}
}
//tutorial 1
//void PhysicsSystem::UpdateObjectAABBs() {
//	gameWorld.OperateOnContents([](GameObject* g) {
//			g->UpdateBroadphaseAABB();
//		}
//	);
//}

//toturial 7 
void PhysicsSystem::UpdateObjectAABBs() {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);
	for (auto i = first; i != last; ++i) {
		(*i)->UpdateBroadphaseAABB();
	}
	
}

/*

This is how we'll be doing collision detection in tutorial 4.
We step thorugh every pair of objects once (the inner for loop offset 
ensures this), and determine whether they collide, and if so, add them
to the collision set for later processing. The set will guarantee that
a particular pair will only be added once, so objects colliding for
multiple frames won't flood the set with duplicates.
*/
void PhysicsSystem::BasicCollisionDetection() {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		if ((*i)->GetPhysicsObject() == nullptr) {
			continue;
		}
		for (auto j = i + 1; j != last; ++j) {
			if ((*j)->GetPhysicsObject() == nullptr) {
				continue;
			}
			CollisionDetection::CollisionInfo info;
			if (CollisionDetection::ObjectIntersection(*i, *j, info)) {
				bool isCoinA = info.a->IsCoin();
				bool isCoinB = info.b->IsCoin();
				bool isPlayerA = info.a->IsPlayer();
				bool isPlayerB = info.b->IsPlayer();
				bool isStartAreaA = info.a->IsStartArea();
				bool isStartAreaB = info.b->IsStartArea();
				bool isTargetA = info.a->IsTarget();
				bool isTargetB = info.b->IsTarget();
				bool isPortal1A = info.a->IsPortal1();
				bool isPortal1B = info.b->IsPortal1();
				bool isPortal2A = info.a->IsPortal2();
				bool isPortal2B = info.b->IsPortal2();
				bool isKeyA = info.a->IsKey();
				bool isKeyB = info.b->IsKey();
				bool isObstalceA = info.a->IsObstacle();
				bool isObstalceB = info.b->IsObstacle();
				bool isEnemy1A = info.a->IsEnemy1();
				bool isEnemy1B = info.b->IsEnemy1();
				bool isGooseA = info.a->IsGoose();
				bool isGooseB = info.b->IsGoose();
				bool isDoor1A = info.a->IsDoor1();
				bool isDoor1B = info.b->IsDoor1();
				bool isobbDoorA = info.a->IsobbDoor();
				bool isobbDoorB = info.b->IsobbDoor();
				if ((isDoor1A && isGooseB) || (isGooseA && isDoor1B)) {
					SetGooseDoor(true);
					if (isGooseB) {
						info.b->GetTransform().SetPosition(Vector3(0, -17, 60));
					}
					if (isGooseA) {
						info.a->GetTransform().SetPosition(Vector3(0, -17, 60));
					}
				}

				if ((isobbDoorA && isPlayerB) || (isobbDoorB && isPlayerA)) {
					if (isobbDoorA) {
						info.a->GetTransform().SetOrientation((Quaternion(Matrix4::Rotation(-90, Vector3(0, 1, 0)))));
					}
					if (isobbDoorB) {
						info.b->GetTransform().SetOrientation((Quaternion(Matrix4::Rotation(-90, Vector3(0, 1, 0)))));
					}
				}

				if ((isPlayerA && isEnemy1B) || (isPlayerB && isEnemy1A)) {
					winorlose--;
				}
				if ((isPlayerA && isObstalceB) || (isObstalceA && isPlayerB)) {
					if (info.a->IsActive() && isObstalceA&&Key>=1) {
						info.a->SetIsActive(false);
						info.a->GetTransform().SetPosition(Vector3(40, -40, -40));
						Key--;
					}
					if (info.b->IsActive() && isObstalceB && Key >= 1) {
						info.b->SetIsActive(false);
						info.b->GetTransform().SetPosition(Vector3(40, -40, -40));
						Key--;
					}
				}
				if ((isKeyA && isPlayerB) || (isKeyB && isPlayerA)) {
					
					if (info.a->IsActive()&&isKeyA){
						info.a->SetIsActive(false);
						Key++;
					}
					if (info.b->IsActive()&&isKeyB) {
						info.b->SetIsActive(false);
						Key++;
					}

				}
				if ((isPortal1A && isPlayerB) || (isPortal1B && isPlayerA)) {
					Debug::Print("you will tp to portal2 when number till 0,number=" + std::to_string(TPtime1), Vector2(0, 50), Debug::RED);
					TPtime1 = TPtime1 - 1;
					if (isPlayerB && TPtime1 == 0) {
						info.b->GetTransform().SetPosition(Vector3(-80, 0, -80));
					}

					if (isPlayerA && TPtime1 == 0) {
						info.a->GetTransform().SetPosition(Vector3(-80, 0, -80));
					}

				}

				if ((isPortal2A && isPlayerB) || (isPortal2B && isPlayerA)) {
					Debug::Print("you will tp to portal1 when number till 0,number=" + std::to_string(TPtime2), Vector2(0, 50), Debug::RED);
					TPtime2 = TPtime2 - 1;
					if (isPlayerB && TPtime2 == 0) {
						info.b->GetTransform().SetPosition(Vector3(-40, 0, 60));
					}
					if (isPlayerA && TPtime2 == 0) {
						info.a->GetTransform().SetPosition(Vector3(-40, 0, 60));
					}
				}
				if ((isStartAreaA && isTargetB) || (isStartAreaB && isTargetA)) {
					if(isTargetA&&info.a->IsActive()){
						winorlose++;
						fraction += 100;
						info.a->SetIsActive(false);
					}
					if (isTargetB && info.b->IsActive()) {
						winorlose++;
						fraction += 100;
						info.b->SetIsActive(false);
					}
					
				}

				if ((isCoinA && isPlayerB) || (isPlayerA && isCoinB)) {
					//info.framesLeft = numCollisionFrames;
					//ImpulseResolveCollision(*info.a, *info.b, info.point);
					//allCollisions.insert(info);//insert into our main set
					if (info.a->IsActive() && isCoinA) {
						info.a->SetIsActive(false);
						/*gameWorld.RemoveGameObject(info.a);*/
						fraction++;
						destroynum--;
					}
					if (info.b->IsActive() && isCoinB) {
						info.b->SetIsActive(false);
						/*gameWorld.RemoveGameObject(info.b);*/
						fraction++;
						destroynum--;
					}
				}
				if ((isCoinA && isGooseB) || (isGooseA && isCoinB)) {
					if (info.a->IsActive() && isCoinA) {
						info.a->SetIsActive(false);
						GooseScore++;
						destroynum--;
					}
					if (info.b->IsActive() && isCoinB) {
						info.b->SetIsActive(false);
						GooseScore++;
						destroynum--;
					}
				}

				if ((isPlayerA && isGooseB) || (isGooseA && isPlayerB)) {
					SetGoosePlayer(true);
					GooseScore += fraction;
					fraction = 0;
					Debug::Print("The goose stole your coins!", Vector2(0, 50), Debug::RED);
				}
				/*std::cout << "Collision between " << (*i)->GetName() << " and " << (*j)->GetName() << std::endl;*/
				ImpulseResolveCollision(*info.a, *info.b, info.point);
				info.framesLeft = numCollisionFrames;
				allCollisions.insert(info);
			}
			
			
		}
		if (winorlose > 0) { 
			mode = 2;
		}
		if (winorlose < 0) { 
			
			mode = 3;
		}
		Debug::Print("Key :" + std::to_string(Key), Vector2(0, 20), Debug::RED);
		Debug::Print("Your fraction is :" + std::to_string(fraction), Vector2(0, 10), Debug::RED);
		Debug::Print("Goose score is :" + std::to_string(GooseScore), Vector2(0, 30), Debug::RED);
		Debug::Print("left destroyable items :" + std::to_string(destroynum), Vector2(50, 10), Debug::RED);
	}
	
	
}

/*

In tutorial 5, we start determining the correct response to a collision,
so that objects separate back out. 

*/
void PhysicsSystem::ImpulseResolveCollision(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const {
	PhysicsObject* physA = a.GetPhysicsObject();
	PhysicsObject* physB = b.GetPhysicsObject();

	Transform& transformA = a.GetTransform();
	Transform& transformB = b.GetTransform();

	float totalMass = physA->GetInverseMass() + physB->GetInverseMass();

	if (totalMass == 0) {
		return; //two static objects
	}

	// separate them out using projection
	transformA.SetPosition(transformA.GetPosition() - (p.normal * p.penetration * (physA->GetInverseMass() / totalMass)));
	transformB.SetPosition(transformB.GetPosition() + (p.normal * p.penetration * (physB->GetInverseMass() / totalMass)));

	Vector3 relativeA = p.localA;
	Vector3 relativeB = p.localB;

	Vector3 angVelocityA = Vector3::Cross(physA->GetAngularVelocity(), relativeA);
	Vector3 angVelocityB = Vector3::Cross(physB->GetAngularVelocity(), relativeB);

	Vector3 fullVelocityA = physA->GetLinearVelocity() + angVelocityA;
	Vector3 fullVelocityB = physB->GetLinearVelocity() + angVelocityB;
	Vector3	contactVelocity=fullVelocityB - fullVelocityA;
	
	float impulseForce = Vector3::Dot(contactVelocity, p.normal);

	//now to work out the effect of inertia....
	Vector3 inertiaA = Vector3::Cross(physA->GetInertiaTensor() * Vector3::Cross(relativeA, p.normal), relativeA);
	Vector3 inertiaB = Vector3::Cross(physB->GetInertiaTensor() * Vector3::Cross(relativeB, p.normal), relativeB);

	float angularEffect = Vector3::Dot(inertiaA + inertiaB, p.normal);

	float cRestitution = 0.66f;//disperse some kinetic energy

	float j = (-(1.0f + cRestitution) * impulseForce) / (totalMass + angularEffect);

	Vector3 fullImpulse = p.normal * j;
	physA->ApplyLinearImpulse(-fullImpulse);
	physB->ApplyLinearImpulse(fullImpulse);

	physA->ApplyAngularImpulse(Vector3::Cross(relativeA, -fullImpulse));
	physB->ApplyAngularImpulse(Vector3::Cross(relativeB, -fullImpulse));


}

/*

Later, we replace the BasicCollisionDetection method with a broadphase
and a narrowphase collision detection method. In the broad phase, we
split the world up using an acceleration structure, so that we can only
compare the collisions that we absolutely need to. 

*/
void PhysicsSystem::BroadPhase() {
	broadphaseCollisions.clear();
	QuadTree <GameObject*> tree(Vector2(1024, 1024), 7, 6);

	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);
	for (auto i = first; i != last; ++i) {
		Vector3 halfSizes;
		if (!(*i)->GetBroadphaseAABB(halfSizes)) {
			continue;
		}
		Vector3 pos = (*i)->GetTransform().GetPosition();
		tree.Insert(*i, pos, halfSizes);
	}
	tree.OperateOnContents([&](std::list<QuadTreeEntry<GameObject*>>& data) {
		CollisionDetection::CollisionInfo info;
		for (auto i = data.begin(); i != data.end(); ++i) {
			for (auto j = std::next(i); j != data.end(); ++j) {
				//is this pair of items already in the collision set-
				//if the same pair is another quadtree node together etc
				info.a = std::min((*i).object, (*j).object);
				info.b = std::max((*i).object, (*j).object);
				broadphaseCollisions.insert(info);
			}
		}
		});
}

/*

The broadphase will now only give us likely collisions, so we can now go through them,
and work out if they are truly colliding, and if so, add them into the main collision list
*/
void PhysicsSystem::NarrowPhase() {
	for (std::set<CollisionDetection::CollisionInfo>::iterator i = broadphaseCollisions.begin(); i != broadphaseCollisions.end(); ++i) {
		CollisionDetection::CollisionInfo info = *i;
		if (CollisionDetection::ObjectIntersection(info.a, info.b, info)) {
			bool isCoinA = info.a->IsCoin();
			bool isCoinB = info.b->IsCoin();
			bool isPlayerA = info.a->IsPlayer();
			bool isPlayerB = info.b->IsPlayer();
			bool isStartAreaA = info.a->IsStartArea();
			bool isStartAreaB = info.b->IsStartArea();
			bool isTargetA = info.a->IsTarget();
			bool isTargetB = info.b->IsTarget();
			bool isPortal1A = info.a->IsPortal1();
			bool isPortal1B= info.b->IsPortal1();
			bool isPortal2A = info.a->IsPortal2();
			bool isPortal2B = info.b->IsPortal2();
			
			if ((isPortal1A && isPlayerB) || (isPortal1B && isPlayerA)) {
				Debug::Print("you will tp to portal2 when number till 0,number=" + std::to_string(TPtime1), Vector2(0, 50), Debug::RED);
				TPtime1 = TPtime1-1;
				if (isPlayerB && TPtime1 == 0) {
					info.b->GetTransform().SetPosition(Vector3(-80, 0, -80));
				}
					
				if (isPlayerA && TPtime1 == 0) {
					info.a->GetTransform().SetPosition(Vector3(-80, 0, -80));
				}
				
			}

			if ((isPortal2A && isPlayerB) || (isPortal2B && isPlayerA)) {
				Debug::Print("you will tp to portal1 when number till 0,number=" + std::to_string(TPtime2), Vector2(0,50), Debug::RED);
				TPtime2 = TPtime2-1;
				if (isPlayerB && TPtime2 == 0) {
					info.b->GetTransform().SetPosition(Vector3(-40, 0, 60));
				}
				if (isPlayerA && TPtime2 == 0) {
					info.a->GetTransform().SetPosition(Vector3(-40, 0, 60));
				}
			}
			if ((isStartAreaA && isTargetB) || (isStartAreaB && isTargetA)) {
				Debug::Print("Your Win!" , Vector2(40, 50), Debug::RED);
			}
			
			if ((isCoinA && isPlayerB) || (isPlayerA && isCoinB)) {
				//info.framesLeft = numCollisionFrames;
				//ImpulseResolveCollision(*info.a, *info.b, info.point);
				//allCollisions.insert(info);//insert into our main set
				if (info.a->IsActive() && isCoinA) {
					info.a->SetIsActive(false);
					/*gameWorld.RemoveGameObject(info.a);*/
					fraction++;
					destroynum--;
				}
				if (info.b->IsActive()&&isCoinB) {
					info.b->SetIsActive(false);
					/*gameWorld.RemoveGameObject(info.b);*/
					fraction++;
					destroynum--;
				}
			}
			
				info.framesLeft = numCollisionFrames;
				ImpulseResolveCollision(*info.a, *info.b, info.point);
				allCollisions.insert(info);//insert into our main set}
			
		}
		Debug::Print("Your fraction is :" + std::to_string(fraction), Vector2(0, 10), Debug::RED);
		Debug::Print("left destroyable items :" + std::to_string(destroynum), Vector2(50, 10), Debug::RED);
	}
}

/*
Integration of acceleration and velocity is split up, so that we can
move objects multiple times during the course of a PhysicsUpdate,
without worrying about repeated forces accumulating etc. 

This function will update both linear and angular acceleration,
based on any forces that have been accumulated in the objects during
the course of the previous game frame.
*/
void PhysicsSystem::IntegrateAccel(float dt) {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);
	for (auto i = first; i != last; ++i) {
		PhysicsObject* object = ((*i))->GetPhysicsObject();
		if (object == nullptr) {
			continue;//No physics object for this GameObject!
		}
		float inverseMass = object->GetInverseMass();
		Vector3 linearVel = object->GetLinearVelocity();
		Vector3 force = object->GetForce();
		Vector3 accel = force * inverseMass;
		if (applyGravity && inverseMass > 0) {
			accel += gravity;//don't move infinitely heavy things
		}
		linearVel += accel * dt;//integrate accel!
		object->SetLinearVelocity(linearVel);

		//Angular stuff
		Vector3 torque = object->GetTorque();
		Vector3 angVel = object->GetAngularVelocity();
		object->UpdateInertiaTensor();//update tensor vs orientation
		Vector3 angAccel = object->GetInertiaTensor() * torque;
		angVel += angAccel * dt;//integrate angular accel!
		object->SetAngularVelocity(angVel);

	}
}

/*
This function integrates linear and angular velocity into
position and orientation. It may be called multiple times
throughout a physics update, to slowly move the objects through
the world, looking for collisions.
*/
void PhysicsSystem::IntegrateVelocity(float dt) {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);
	float frameLinearDamping = 1.0f - (0.4f * dt);

	for (auto i = first; i != last; ++i) {
		PhysicsObject* object = (*i)->GetPhysicsObject();
		if (object == nullptr) {
			continue;
		}
		Transform& transform = (*i)->GetTransform();
		//Position stuff
		Vector3 position = transform.GetPosition();
		Vector3 linearVel = object->GetLinearVelocity();
		position += linearVel * dt;
		transform.SetPosition(position);
		//Linear Damping
		linearVel = linearVel * frameLinearDamping;
		if (object->GetInverseMass() != 0.0f && position.y <= 0.0f) {
			float frictional = 0.01f / object->GetInverseMass();
			Vector3 ds = linearVel.Normalised();
			ds = ds * frictional;
			if (linearVel.Length() > ds.Length()) {
				linearVel = linearVel - ds;
			}
			else {
				linearVel = Vector3(0, 0, 0);
			}
		}
		object->SetLinearVelocity(linearVel);

		//Orientation stuff
		Quaternion orientation = transform.GetOrientation();
		Vector3 angVel = object->GetAngularVelocity();

		orientation = orientation + (Quaternion(angVel * dt * 0.5f, 0.0f) * orientation);
		orientation.Normalise();

		transform.SetOrientation(orientation);

		//Damp the angular velocity too
		float frameAngularDamping = 1.0f - (0.4f * dt);
		angVel = angVel * frameAngularDamping;
		object->SetAngularVelocity(angVel);
		
	}
}

/*
Once we're finished with a physics update, we have to
clear out any accumulated forces, ready to receive new
ones in the next 'game' frame.
*/
void PhysicsSystem::ClearForces() {
	gameWorld.OperateOnContents(
		[](GameObject* o) {
			o->GetPhysicsObject()->ClearForces();
		}
	);
}


/*

As part of the final physics tutorials, we add in the ability
to constrain objects based on some extra calculation, allowing
us to model springs and ropes etc. 

*/
void PhysicsSystem::UpdateConstraints(float dt) {
	std::vector<Constraint*>::const_iterator first;
	std::vector<Constraint*>::const_iterator last;
	gameWorld.GetConstraintIterators(first, last);

	for (auto i = first; i != last; ++i) {
		(*i)->UpdateConstraint(dt);
	}
}