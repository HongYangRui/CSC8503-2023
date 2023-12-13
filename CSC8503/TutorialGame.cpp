#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"

#include "PositionConstraint.h"
#include "OrientationConstraint.h"
#include "StateGameObject.h"
#include"HingeConstraint.h"
#include"HeightConstraint.h"
#include <NavigationGrid.h>



using namespace NCL;
using namespace CSC8503;
vector<Vector3> testNodes;
TutorialGame::TutorialGame() : controller(*Window::GetWindow()->GetKeyboard(), *Window::GetWindow()->GetMouse()) {
	world		= new GameWorld();
#ifdef USEVULKAN
	renderer	= new GameTechVulkanRenderer(*world);
	renderer->Init();
	renderer->InitStructures();
#else 
	renderer = new GameTechRenderer(*world);
#endif

	physics		= new PhysicsSystem(*world);

	forceMagnitude	= 10.0f;
	useGravity		= false;
	inSelectionMode = false;

	world->GetMainCamera().SetController(controller);

	controller.MapAxis(0, "Sidestep");
	controller.MapAxis(1, "UpDown");
	controller.MapAxis(2, "Forward");

	controller.MapAxis(3, "XLook");
	controller.MapAxis(4, "YLook");

	InitialiseAssets();
}




/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	cubeMesh	= renderer->LoadMesh("cube.msh");
	sphereMesh	= renderer->LoadMesh("sphere.msh");
	charMesh	= renderer->LoadMesh("goat.msh");
	enemyMesh	= renderer->LoadMesh("Keeper.msh");
	bonusMesh	= renderer->LoadMesh("coin.msh");
	capsuleMesh = renderer->LoadMesh("capsule.msh");
	GooseMesh = renderer->LoadMesh("goose.msh");
	basicTex	= renderer->LoadTexture("checkerboard.png");
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");

	InitCamera();
	InitWorld();
}

TutorialGame::~TutorialGame()	{
	delete cubeMesh;
	delete sphereMesh;
	delete charMesh;
	delete enemyMesh;
	delete bonusMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;
}
void TutorialGame::UPdateConstarin() {
	Vector3 position1 = player->GetTransform().GetPosition();
	Vector3 position2 = target->GetTransform().GetPosition();
	Vector3 diff = position1 - position2;
	float distance = diff.Length();
	if (distance < 4.0f) {
		PositionConstraint* constraint = new PositionConstraint(player, target, 4);
		world->AddConstraint(constraint);
		target->GetRenderObject()->SetColour(Debug::GREEN);
	}
}

void TutorialGame::UpdateGame(float dt) {
	UPdateConstarin();
	totaltime += dt;
	Debug::Print("Spent time:" + std::to_string(totaltime), Vector2(60, 20), Debug::RED);
	//statemachine
	/*GoosePathfinding();*/

	if (state == Ongoing) {
		state = rootSequence->Execute(1.0f);//fake dt
	}

	if (Enemy1) {
		Enemy1->Update(dt);
	}

	if (!inSelectionMode) {
		world->GetMainCamera().UpdateCamera(dt);
	}
	if (lockedObject != nullptr) {
		Vector3 objPos = lockedObject->GetTransform().GetPosition();
		Vector3 camPos = objPos + lockedOffset;

		Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0,1,0));

		Matrix4 modelMat = temp.Inverse();

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera().SetPosition(camPos);
		world->GetMainCamera().SetPitch(angles.x);
		world->GetMainCamera().SetYaw(angles.y);
	}

	UpdateKeys();

	if (useGravity) {
		Debug::Print("(G)ravity on", Vector2(5, 95), Debug::RED);
	}
	else {
		Debug::Print("(G)ravity off", Vector2(5, 95), Debug::RED);
	}

	RayCollision closestCollision;
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::K) && selectionObject) {
		Vector3 rayPos;
		Vector3 rayDir;

		rayDir = selectionObject->GetTransform().GetOrientation() * Vector3(0, 0, -1);

		rayPos = selectionObject->GetTransform().GetPosition();

		Ray r = Ray(rayPos, rayDir);

		if (world->Raycast(r, closestCollision, true, selectionObject)) {
			if (objClosest) {
				objClosest->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
			}
			objClosest = (GameObject*)closestCollision.node;

			objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
		}
	}
	    Vector3 AiPos;
		Vector3 AiDir;
		AiDir = Goose->GetTransform().GetOrientation()*Vector3(1,1,1);
		AiPos=Goose-> GetTransform().GetPosition();
		Ray ai = Ray(AiPos,AiDir);
		if(world->Raycast(ai,closestCollision,true, target)) {
			if (objClosest) {
				objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
			}
			objClosest = (GameObject*)closestCollision.node;
			objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
		}
		Debug::DrawLine(AiPos, player->GetTransform().GetPosition(), Vector4(1,0,1,1));

	Debug::DrawLine(Vector3(), Vector3(0, 100, 0), Vector4(1, 0, 0, 1));
	//Debug::DrawLine(Vector3(), Vector3(100, 0, 0), Vector4(0, 1, 0, 1));
	//Debug::DrawLine(Vector3(), Vector3(0, 0, 100), Vector4(0, 0, 1, 1));
	SelectObject();
	MoveSelectedObject();

	world->UpdateWorld(dt);
	renderer->Update(dt);
	physics->Update(dt);

	renderer->Render();
	Debug::UpdateRenderables(dt);
	
}

void TutorialGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F8)) {
		world->ShuffleObjects(false);
	}

	if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}

	
}

void TutorialGame::LockedObjectMovement() {
	Matrix4 view		= world->GetMainCamera().BuildViewMatrix();
	Matrix4 camWorld	= view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();


	if (Window::GetKeyboard()->KeyDown(KeyCodes::W)) { 
		selectionObject->GetPhysicsObject()->AddForce(fwdAxis*10);
		selectionObject->GetTransform().SetOrientation(Quaternion(Matrix4::Rotation(0, Vector3(0, 1, 0))));
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::S)) {
		selectionObject->GetPhysicsObject()->AddForce(-fwdAxis*10);
		selectionObject->GetTransform().SetOrientation(Quaternion(Matrix4::Rotation(-180, Vector3(0, 1, 0))));
	}
	if (Window::GetKeyboard()->KeyDown(KeyCodes::A)) {
		selectionObject->GetPhysicsObject()->AddForce(-rightAxis*10);
		selectionObject->GetTransform().SetOrientation(Quaternion(Matrix4::Rotation(90, Vector3(0, 1, 0))));
		
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::D)) {
		selectionObject->GetPhysicsObject()->AddForce(rightAxis*10);
		selectionObject->GetTransform().SetOrientation(Quaternion(Matrix4::Rotation(-90, Vector3(0, 1, 0))));
	}
	if (Window::GetKeyboard()->KeyDown(KeyCodes::NEXT)) {
		selectionObject->GetPhysicsObject()->AddForce(Vector3(0,-10,0));
	}

	
	
}

void TutorialGame::DebugObjectMovement() {
//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyCodes::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
			
		}
		if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
			
		}
		if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
			
		}
		if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
			
		}
		if (Window::GetKeyboard()->KeyDown(KeyCodes::W)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
			selectionObject->GetTransform().SetOrientation(Quaternion(Matrix4::Rotation(0, Vector3(0, 1, 0))));
			
		}
		if (Window::GetKeyboard()->KeyDown(KeyCodes::S)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
			selectionObject->GetTransform().SetOrientation(Quaternion(Matrix4::Rotation(-180, Vector3(0, 1, 0))));
			
		}
		if (Window::GetKeyboard()->KeyDown(KeyCodes::D)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(10, 0, 0));
			selectionObject->GetTransform().SetOrientation(Quaternion(Matrix4::Rotation(-90, Vector3(0, 1, 0))));
		}
		if (Window::GetKeyboard()->KeyDown(KeyCodes::A)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(-10, 0, 0));
			selectionObject->GetTransform().SetOrientation(Quaternion(Matrix4::Rotation(90, Vector3(0, 1, 0))));
			
		}
		if (Window::GetKeyboard()->KeyDown(KeyCodes::SHIFT)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 1000, 0));
		}
	}
}



void TutorialGame::InitCamera() {
	world->GetMainCamera().SetNearPlane(0.1f);
	world->GetMainCamera().SetFarPlane(400.0f);
	world->GetMainCamera().SetPitch(-70.0f);
	world->GetMainCamera().SetYaw(360.0f);
	world->GetMainCamera().SetPosition(Vector3(0, 250, 100));
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	world->ClearAndErase();
	physics->Clear();

	/*InitMixedGridWorld(15, 15, 3.5f, 3.5f);*/

	InitGameExamples();
	InitDefaultFloor();
	/*InitGameWorld();*/
	GenerateMazeBuilding();
	SetStartArea();
	SetPortal1();
	SetPortal2();
	/*BridgeConstraintTest();*/
	GooseBehaviourTree();
}

/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position) {
	GameObject* floor = new GameObject();

	Vector3 floorSize = Vector3(200, 2, 200);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize )
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}
GameObject* TutorialGame::SetStartArea() {
	startArea = new GameObject();

	Vector3 floorSize = Vector3(20, 1, 20);
	AABBVolume* volume = new AABBVolume(floorSize/2);
	startArea->SetBoundingVolume((CollisionVolume*)volume);
	startArea->GetTransform()
		.SetScale(floorSize)
		.SetPosition(Vector3(0,-18,80));

	startArea->SetRenderObject(new RenderObject(&startArea->GetTransform(), cubeMesh, basicTex, basicShader));
	startArea->SetPhysicsObject(new PhysicsObject(&startArea->GetTransform(), startArea->GetBoundingVolume()));

	startArea->GetPhysicsObject()->SetInverseMass(0);
	startArea->GetPhysicsObject()->InitCubeInertia();
	startArea->GetRenderObject()->SetColour(Debug::CYAN);
	startArea->SetIsStartArea(true);
	world->AddGameObject(startArea);

	return startArea;
}


GameObject* TutorialGame::SetPortal1() {
	portal1 = new GameObject();
	Vector3 floorSize = Vector3(10, 1, 10);
	AABBVolume* volume = new AABBVolume(floorSize/2);
	portal1->SetBoundingVolume((CollisionVolume*)volume);
	portal1->GetTransform()
		.SetScale(floorSize)
		.SetPosition(Vector3(-40, -18, 60));

	portal1->SetRenderObject(new RenderObject(&portal1->GetTransform(), cubeMesh, basicTex, basicShader));
	portal1->SetPhysicsObject(new PhysicsObject(&portal1->GetTransform(), portal1->GetBoundingVolume()));

	portal1->GetPhysicsObject()->SetInverseMass(0);
	portal1->GetPhysicsObject()->InitCubeInertia();
	portal1->GetRenderObject()->SetColour(Debug::BLUE);
	portal1->SetIsPortal1(true);
	world->AddGameObject(portal1);

	return portal1;
}

GameObject* TutorialGame::SetPortal2() {
	portal2 = new GameObject();
	Vector3 floorSize = Vector3(10, 1, 10);
	AABBVolume* volume = new AABBVolume(floorSize/2);
	portal2->SetBoundingVolume((CollisionVolume*)volume);
	portal2->GetTransform()
		.SetScale(floorSize)
		.SetPosition(Vector3(-80, -18,-80 ));

	portal2->SetRenderObject(new RenderObject(&portal2->GetTransform(), cubeMesh, basicTex, basicShader));
	portal2->SetPhysicsObject(new PhysicsObject(&portal2->GetTransform(), portal2->GetBoundingVolume()));

	portal2->GetPhysicsObject()->SetInverseMass(0);
	portal2->GetPhysicsObject()->InitCubeInertia();
	portal2->GetRenderObject()->SetColour(Debug::BLUE);
	portal2->SetIsPortal2(true);
	world->AddGameObject(portal2);

	return portal2;
}

GameObject* TutorialGame::AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass) {
	GameObject* capsule = new GameObject();

	CapsuleVolume* volume = new CapsuleVolume(halfHeight, radius);
	capsule->SetBoundingVolume((CollisionVolume*)volume); 

	capsule->GetTransform()
		.SetScale(Vector3(radius * 2, halfHeight, radius * 2))
		.SetPosition(position);

	capsule->SetRenderObject(new RenderObject(&capsule->GetTransform(), capsuleMesh, basicTex, basicShader));
	capsule->SetPhysicsObject(new PhysicsObject(&capsule->GetTransform(), capsule->GetBoundingVolume()));

	capsule->GetPhysicsObject()->SetInverseMass(inverseMass);
	capsule->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(capsule);

	return capsule;

}


/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new GameObject();

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddOBBCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();

	OBBVolume* volume = new OBBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position) {
	float meshSize		= 2.0f;
	float inverseMass	= 0.5f;

	GameObject* character = new GameObject();
	SphereVolume* volume  = new SphereVolume(1.0f);

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), charMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();
	character->GetRenderObject()->SetColour(Debug::BLACK);
	world->AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddGooseToWorld(const Vector3& position) {
	float meshSize = 2.0f;
	float inverseMass = 0.5f;

	GameObject* Goose = new GameObject();
	SphereVolume* volume = new SphereVolume(1.0f);

	Goose->SetBoundingVolume((CollisionVolume*)volume);

	Goose->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	Goose->SetRenderObject(new RenderObject(&Goose->GetTransform(), GooseMesh, nullptr, basicShader));
	Goose->SetPhysicsObject(new PhysicsObject(&Goose->GetTransform(), Goose->GetBoundingVolume()));

	Goose->GetPhysicsObject()->SetInverseMass(inverseMass);
	Goose->GetPhysicsObject()->InitSphereInertia();
	Goose->GetRenderObject()->SetColour(Debug::RED);
	world->AddGameObject(Goose);

	return Goose;
}

GameObject* TutorialGame::AddEnemyToWorld(const Vector3& position) {
	float meshSize		= 3.0f;
	float inverseMass	= 0.5f;

	GameObject* character = new GameObject();

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();
	character->GetRenderObject()->SetColour(Debug::RED);
	world->AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddBonusToWorld(const Vector3& position) {
	GameObject* coin = new GameObject();

	SphereVolume* volume = new SphereVolume(1.0f);
	coin->SetBoundingVolume((CollisionVolume*)volume);
	coin->GetTransform()
		.SetScale(Vector3(0.4f, 0.4f, 0.4f))
		.SetPosition(position);

	coin->SetRenderObject(new RenderObject(&coin->GetTransform(), bonusMesh, nullptr, basicShader));
	coin->SetPhysicsObject(new PhysicsObject(&coin->GetTransform(), coin->GetBoundingVolume()));

	coin->GetPhysicsObject()->SetInverseMass(0.5f);
	coin->GetPhysicsObject()->InitSphereInertia();
	coin->GetRenderObject()->SetColour(Debug::YELLOW);
	coin->SetIsCoin(true);
	world->AddGameObject(coin);

	return coin;
}

void TutorialGame::UpdateColor(GameObject* object, const Vector4& colour) {
	object->GetRenderObject()->SetColour(colour);
}

StateGameObject* TutorialGame::AddStateObjectToWorld(const Vector3& position)
{
	float meshSize = 3.0f;
	float inverseMass = 0.5f;
	StateGameObject* robot = new StateGameObject();
	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	robot->SetBoundingVolume((CollisionVolume*)volume);
	robot->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	robot->SetRenderObject(new RenderObject(&robot->GetTransform(), enemyMesh, nullptr, basicShader));
	robot->SetPhysicsObject(new PhysicsObject(&robot->GetTransform(), robot->GetBoundingVolume()));

	robot->GetPhysicsObject()->SetInverseMass(inverseMass);
	robot->GetPhysicsObject()->InitSphereInertia();
	robot->GetRenderObject()->SetColour(Debug::RED);

	world->AddGameObject(robot);

	return robot;
}



void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(0, -20, 0));
}


void TutorialGame::InitGameExamples() {
	player= AddPlayerToWorld(Vector3(0, -17, 80));
	player->SetIsPlayer(true);
	target = AddPlayerToWorld(Vector3(-40, -17, 20));
	target->SetIsTarget(true);
	/*AddPlayerToWorld(Vector3(0, -17, 80));*/
	/*AddEnemyToWorld(Vector3(5, 5, 0));*/
	obbdoor=AddOBBCubeToWorld(Vector3(10, -10, 20), Vector3(1, 10, 10), 0.0f);
	obbdoor->SetIsobbDoor(true);
	door1a=AddCubeToWorld(Vector3(32, -8, 0), Vector3(1, 10, 1), 0.0f);
	door1b=AddCubeToWorld(Vector3(41, -8, 0), Vector3(8, 10, 1), 1.0f);
	door1b->SetIsDoor1(true);
	PositionConstraint* DoorConstraint = new PositionConstraint(door1a, door1b, 9);
	HingeConstraint* doorConstraint = new HingeConstraint(door1a,door1b);
	HeightConstraint* heightConstraint = new HeightConstraint(door1b,-8.0f);
	world->AddConstraint(heightConstraint);
	world->AddConstraint(DoorConstraint);
	world->AddConstraint(doorConstraint);
	Obstacle = AddSphereToWorld(Vector3(0, -10, 40), 10.0f, 0.0f);
	Obstacle->GetRenderObject()->SetColour(Debug::MAGENTA);
	Obstacle->SetIsObstacle(true);
	Key = AddCapsuleToWorld(Vector3(-80, -14, 0), 3.0f, 1.0f, 1.0f);
	Key->GetRenderObject()->SetColour(Debug::MAGENTA);
	Key->SetIsKey(true);
	Goose = AddGooseToWorld(Vector3(80,-17,20));
	Goose->SetIsGoose(true);
	OrientationConstraint* gtConstraint = new OrientationConstraint(Goose, player);
	world->AddConstraint(gtConstraint);
	Enemy1 = AddStateObjectToWorld(Vector3(-82, -14,0));
	Enemy1->SetIsEnemy1(true);
	/*OrientationConstraint* guradConstaint = new OrientationConstraint(Enemy1,Key);
	world->AddConstraint(guradConstaint);*/


	AddCubeToWorld(Vector3(60, -15, 60), Vector3(1, 1, 1), 1.0f);
	AddCubeToWorld(Vector3(60, -15, 65), Vector3(1, 1, 1), 1.0f);
	AddSphereToWorld(Vector3(65, -15, 60), 1.0f, 1.0f);
	AddSphereToWorld(Vector3(65, -15, 65), 1.0f, 1.0f);
	AddCapsuleToWorld(Vector3(70, -15,60), 2.0f, 1.0f, 1.0f);
	AddCapsuleToWorld(Vector3(70, -15, 65), 2.0f, 1.0f, 1.0f);
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims);
			}
			else {
				AddSphereToWorld(position, sphereRadius);
			}
		}
	}
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols+1; ++x) {
		for (int z = 1; z < numRows+1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
}

//void TutorialGame::InitGameWorld() {
//	//edge
//	AddCubeToWorld(Vector3(0, 0, 200), Vector3(200, 20, 1), 0.0f);
//	AddCubeToWorld(Vector3(0, 0, -200), Vector3(200, 20, 1), 0.0f);
//	AddCubeToWorld(Vector3(200, 0, 0), Vector3(1, 20, 200), 0.0f);
//	AddCubeToWorld(Vector3(-200, 0, 0), Vector3(1, 20, 200), 0.0f);
//	//center
//	AddCubeToWorld(Vector3(-45, 0, 25), Vector3(1, 20, 20), 0.0f);
//	AddCubeToWorld(Vector3(-45, 0, -25), Vector3(1, 20, 20), 0.0f);
//	AddCubeToWorld(Vector3(45, 0, 25), Vector3(1, 20, 20), 0.0f);
//	AddCubeToWorld(Vector3(45, 0, -25), Vector3(1, 20, 20), 0.0f);
//	AddCubeToWorld(Vector3(25, 0, -44), Vector3(19, 20, 1), 0.0f);
//	AddCubeToWorld(Vector3(-25, 0, -44 ), Vector3(19, 20, 1), 0.0f);
//	AddCubeToWorld(Vector3(25, 0, 44), Vector3(19, 20, 1), 0.0f);
//	AddCubeToWorld(Vector3(-25, 0, 44 ), Vector3(19, 20, 1), 0.0f);
//	
//	AddCubeToWorld(Vector3(-100, 0, 100), Vector3(1, 20, 100), 0.0f);
//	AddCubeToWorld(Vector3(100, 0, -100), Vector3(1, 20, 100), 0.0f);
//
//	AddCubeToWorld(Vector3(50, 0, -88), Vector3(50, 20, 1), 0.0f);
//	AddCubeToWorld(Vector3(-50, 0, 88), Vector3(50, 20, 1), 0.0f);
//
//	AddCubeToWorld(Vector3(-100, 0, -150), Vector3(100, 20, 1), 0.0f);
//	AddCubeToWorld(Vector3(100, 0, 150), Vector3(100, 20, 1), 0.0f);
//
//	AddCubeToWorld(Vector3(-122, 10, -25), Vector3(77, 20, 1), 0.0f);
//	AddCubeToWorld(Vector3(122, 10, 25), Vector3(77, 20, 1), 0.0f);
//
//	AddCubeToWorld(Vector3(0, -15, 45), Vector3(5, 5, 5), 0.5f);
//	AddCubeToWorld(Vector3(-45, -15, 0), Vector3(5, 5, 5), 0.5f);
//	
//	AddSphereToWorld(Vector3(-100, -20, -100), 40.0f,0.0f);
//	AddSphereToWorld(Vector3(100, -20, 100), 40.0f,0.0f);
//}

void TutorialGame::GenerateMazeBuilding() {
	std::vector<std::string> mazeBlueprint = { 
		"11111111111",
		"10101222221", 
		"10101211121", 
		"10101200121", 
		"10122210121",
		"10121110121", 
		"10101002101", 
		"10111011111", 
		"10120000001",
		"10122000001",
		"11111111111"
	};
	float cellSize = 10.0f; float wallThickness = 10.0f;
	for (int i = 0; i < mazeBlueprint.size(); ++i) {
		for (int j = 0; j < mazeBlueprint[i].size(); ++j) {
			if (mazeBlueprint[i][j] == '1') {
				Vector3 position(j * cellSize*2-100, -18.0f + wallThickness, i * cellSize*2-100);
				AddCubeToWorld(position, Vector3(cellSize, wallThickness, cellSize), 0.0f);
			}
			if (mazeBlueprint[i][j] == '2') {
				Vector3 position(j * cellSize * 2 - 100, -17.0f, i * cellSize * 2 - 100);
				AddBonusToWorld(position);
				
			}
			else if (mazeBlueprint[i][j] == '0') {}
		}
	}
}

void TutorialGame::GoosePathfinding() {
	/*NavigationGrid grid("GooseGrid1.txt");*/
	NavigationGrid grid("GooseGrid2.txt");
	NavigationPath outPath;
	/*Vector3 offset = Vector3(40, 20, 80);*/
	Vector3 offset = Vector3(100, 15,100);
	Vector3 startPos = Goose->GetTransform().GetPosition();
	/*Vector3 endPos = target->GetTransform().GetPosition();*/
	Vector3 endPos = player->GetTransform().GetPosition();
	bool found = grid.FindPath(startPos + offset, endPos + offset, outPath);
	Vector3 pos;
	if (found) {
		while (outPath.PopWaypoint(pos)) {
			testNodes.push_back(pos);
		}
		if (testNodes.size()>1) {
			Goose->GetPhysicsObject()->AddForce(((testNodes[1] - offset) - startPos));
		}
	}
		for (int i = 1; i < testNodes.size(); ++i) {
			Vector3 a = testNodes[i - 1] - offset;
			Vector3 b = testNodes[i] - offset;
			Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
			/*Goose->GetPhysicsObject()->AddForce((b-a));*/
		}
		testNodes.clear();
}


void TutorialGame::GoosePathfindingDoor() {
	/*NavigationGrid grid("GooseGrid1.txt");*/
	NavigationGrid grid("GooseGrid2.txt");
	NavigationPath outPath;
	/*Vector3 offset = Vector3(40, 20, 80);*/
	Vector3 offset = Vector3(100, 15, 100);
	Vector3 startPos = Goose->GetTransform().GetPosition();
	/*Vector3 endPos = target->GetTransform().GetPosition();*/
	Vector3 endPos = Vector3(41, -8, 0);
	bool found = grid.FindPath(startPos + offset, endPos + offset, outPath);
	Vector3 pos;
	if (found) {
		while (outPath.PopWaypoint(pos)) {
			testNodes.push_back(pos);
		}
		if (testNodes.size() > 1) {
			Goose->GetPhysicsObject()->AddForce(((testNodes[1] - offset) - startPos));
		}
	}
	for (int i = 1; i < testNodes.size(); ++i) {
		Vector3 a = testNodes[i - 1] - offset;
		Vector3 b = testNodes[i] - offset;
		Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
		/*Goose->GetPhysicsObject()->AddForce((b-a));*/
	}
	testNodes.clear();
}
/*
Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be 
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around. 

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		Debug::Print("Press Q to change to camera mode!", Vector2(5, 85));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::Left)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;

				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				return true;
			}
			else {
				return false;
			}
		}
		if (Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::L)) {
			if (selectionObject) {
				if (lockedObject == selectionObject) {
					lockedObject = nullptr;
				}
				else {
					lockedObject = selectionObject;
				}
			}
		}
	}
	else {
		Debug::Print("Press Q to change to select mode!", Vector2(5, 85));
	}
	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/

void TutorialGame::MoveSelectedObject() {
	Debug::Print("Click Force:" + std::to_string(forceMagnitude), Vector2(5, 90));
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

	if (!selectionObject) {
		return;//we haven't selected anything!
	}
	//Push the selected object!
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::Right)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());

		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude*100, closestCollision.collidedAt);
			}
		}
	}
}

//constraints
void TutorialGame::BridgeConstraintTest() {
	Vector3 cubeSize = Vector3(8, 8, 8);
	float invCubeMass = 5;//how heavy the middle pieces are
	int numLinks = 10;
	float maxDistance = 30;//constraint distance
	float cubeDistance = 20;//distance between links

	Vector3 startPos = Vector3(30, 100, 30);

	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);
	GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), cubeSize, 0);
	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);
		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;
	}
	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);
}


void TutorialGame::GooseBehaviourTree() {
	float behaviourTimer;
	float distanceTotarget;
	BehaviourAction* sleep = new BehaviourAction("Sleep", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			behaviourTimer = 100;
			std::cout << "sleep!\n";
			Debug::Print("Sleep:", Goose->GetTransform().GetPosition());
			state = Ongoing;
		}
		else if (state == Ongoing) {
			behaviourTimer -= dt;
			Debug::Print("Awake!", Goose->GetTransform().GetPosition());
			if (behaviourTimer <= 0.0f) {
				std::cout << "awake!\n";
				
				return Success;
			}
		}
		return state;//will be ongoing until success
		});
	BehaviourAction* goToDoor = new BehaviourAction("gotodoor", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			std::cout << "Going to the door\n";
			Debug::Print("Going to the door", Goose->GetTransform().GetPosition());
			state = Ongoing;
		}
		else if (state == Ongoing) {
			GoosePathfindingDoor();
			while (physics->IsGooseDoor()) {
				std::cout << "Going to the door!\n";
				return Success;
			}
		}
		return state;//will be ongoing until success
		});
	BehaviourAction* bypassDoor = new BehaviourAction("Bypass Door", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			std::cout << "Bypass Door\n";
			Debug::Print("Bypass Door!", Goose->GetTransform().GetPosition());
			state == Ongoing;
		}
		if (state == Ongoing) {
			Goose->GetTransform().SetPosition(Vector3(0, -17, 80));
			std::cout << "Bypass Door!\n";
			return Success;
		}
		return state;
		});
	BehaviourAction* speak = new BehaviourAction("speak", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			Debug::Print("I will catch you!", Goose->GetTransform().GetPosition());
			std::cout << "I will catch you!\n";
			return Ongoing;
		}
		else if (state == Ongoing) {
			bool found = rand() % 2;
			if (found) {
				std::cout << "I will catch you!\n";
				Debug::Print("I will catch you!", Goose->GetTransform().GetPosition());
				return Failure;
			}
		}
		return state;
		});
	BehaviourAction* chase = new BehaviourAction("chase player", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			std::cout << "Chasing player!\n";
			Debug::Print("Chasing player!", Goose->GetTransform().GetPosition());
			return Ongoing;
		}
		else if (state == Ongoing) {
			GoosePathfinding();
			while (physics->IsGoosePlayer()) {
				std::cout << "Catch you!\n";
				return Failure;
			}
		}
		return state;
		});
	BehaviourSequence* sequence = new BehaviourSequence("Room Sequence");
	sequence->AddChild(sleep);
	sequence->AddChild(goToDoor);
	sequence->AddChild(bypassDoor);
	BehaviourSelector* selection = new BehaviourSelector("Loot Selection");
	selection->AddChild(speak);
	selection->AddChild(chase);
	rootSequence = new BehaviourSequence("Root Sequence");
	rootSequence->AddChild(sequence);
	rootSequence->AddChild(selection);
	//for (int i = 0; i < 5; ++i) {
		rootSequence->Reset();
		state = Ongoing;
		/*behaviourTimer = 0.0f;
		distanceTotarget = rand() % 250;*/
		//BehaviourState state = Ongoing;
		//std::cout << "We are going on an adventure!\n";
		//if (state == Ongoing) {
		//	state = rootSequence->Execute(1.0f);//fake dt
		//}
	/*	if (state == Failure) {
			std::cout << "What a waste of time!\n";
		}*/
//	}
	//std::cout << "All done!\n";
}