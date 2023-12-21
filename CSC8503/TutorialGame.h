#include "../NCLCoreClasses/KeyboardMouseController.h"

#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"

#include "StateGameObject.h"
#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"
#include"PositionConstraint.h"

namespace NCL {
	namespace CSC8503 {
		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();
			virtual void UpdateGame(float dt);
			BehaviourSequence* rootSequence; 
			bool isTimeMode = false;
		protected:
			//statemachine
			StateGameObject* AddStateObjectToWorld(const Vector3& position);
			StateGameObject* Enemy1;

			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();

			void InitWorld();

			void UPdateConstarin();
			

			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on). 
			*/
			void InitGameExamples();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);
			//create game scene
			void InitGameWorld();
			void GenerateMazeBuilding();

			void GoosePathfinding();

			void GoosePathfindingDoor();

			void InitDefaultFloor();

			void MoveObject(const Vector3& direction);
			void UpdateObjectOrientation(const Vector3& direction);

			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();
			void BridgeConstraintTest();
			void GooseBehaviourTree();
			void UpdateObjectRotation(GameObject* obj);
			void GrapplingHook();
			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* SetStartArea();
			GameObject* SetPortal1();
			GameObject* SetPortal2();
			GameObject* AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddOBBCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);

			GameObject* AddPlayerToWorld(const Vector3& position);
			GameObject* AddGooseToWorld(const Vector3& position);
			GameObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position);

			void UpdateColor(GameObject* object, const Vector4& colour);

#ifdef USEVULKAN
			GameTechVulkanRenderer*	renderer;
#else
			GameTechRenderer* renderer;
#endif
			PhysicsSystem*		physics;
			GameWorld*			world;

			KeyboardMouseController controller;

			bool useGravity;
			bool inSelectionMode;

			float		forceMagnitude;

			GameObject* selectionObject = nullptr;

			Mesh*	capsuleMesh = nullptr;
			Mesh*	cubeMesh	= nullptr;
			Mesh*	sphereMesh	= nullptr;
			Mesh* GooseMesh = nullptr;


			Texture*	basicTex	= nullptr;
			Shader*		basicShader = nullptr;

			//Coursework Meshes
			Mesh*	charMesh	= nullptr;
			Mesh*	enemyMesh	= nullptr;
			Mesh*	bonusMesh	= nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 40, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

			GameObject* objClosest = nullptr;
			GameObject* obbdoor;
			GameObject* player;
			GameObject* target;
			GameObject* startArea;
			GameObject* door1a,*door1b;
			GameObject* Obstacle;
			GameObject* Key;
			GameObject* portal1;
			GameObject* portal2;
			GameObject* Goose;
			GameObject* OBB1;
			GameObject* OBB2;
			/*bool gameEnded = false;*/
			float totaltime = 0.0f;
			BehaviourState state;
			bool p = true;
			
		};
	}
}

