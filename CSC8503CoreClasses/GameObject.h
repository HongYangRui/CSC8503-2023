#pragma once
#include "Transform.h"
#include "CollisionVolume.h"

using std::vector;

namespace NCL::CSC8503 {
	class NetworkObject;
	class RenderObject;
	class PhysicsObject;

	class GameObject	{
	public:
		GameObject(const std::string& name = "");
		~GameObject();

		void SetBoundingVolume(CollisionVolume* vol) {
			boundingVolume = vol;
		}

		const CollisionVolume* GetBoundingVolume() const {
			return boundingVolume;
		}

		bool IsActive() const {
			return isActive;
		}
		bool IsCoin() const {
			return isCoin;
		}
		void SetIsCoin(bool value){
			isCoin = value;
		}
		bool IsPlayer() const {
			return isPlayer;
		}
		void SetIsPlayer(bool value) {
			isPlayer = value;
		}

		void SetIsActive(bool value) {
			isActive = value;
			if (value == false) {
				SetBoundingVolume(NULL);
			}
		}

		bool IsTarget()const {
			return isTarget;
		}
		void SetIsTarget(bool value) {
			isTarget = value;
		}
		bool IsStartArea()const {
			return isStartArea;
		}
		void SetIsStartArea(bool value) {
			isStartArea = value;
		}
		bool IsPortal1()const {
			return isPortal1;
		}
		void SetIsPortal1(bool value) {
			isPortal1 = value;
		}

		bool IsPortal2()const {
			return isPortal2;
		}
		void SetIsPortal2(bool value) {
			isPortal2 = value;
		}
		bool IsObstacle()const {
			return isObstacle;
		}
		void SetIsObstacle(bool value) {
			isObstacle = value;
		}
		bool IsKey()const {
			return isKey;
		}
		void SetIsKey(bool value) {
			isKey = value;
		}
		bool IsEnemy1()const {
			return isEnemy1;
		}
		void SetIsEnemy1(bool value) {
			isEnemy1 = value;
		}
		bool IsGoose()const {
			return isGoose;
		}
		void SetIsGoose(bool value) {
			isGoose = value;
		}
		bool IsDoor1()const {
			return isDoor1;
		}
		void SetIsDoor1(bool value) {
			isDoor1 = value;
		}
		bool IsobbDoor()const {
			return isobbDoor;
		}
		void SetIsobbDoor(bool value) {
			isobbDoor = value;
		}


		Transform& GetTransform() {
			return transform;
		}

		RenderObject* GetRenderObject() const {
			return renderObject;
		}

		PhysicsObject* GetPhysicsObject() const {
			return physicsObject;
		}

		NetworkObject* GetNetworkObject() const {
			return networkObject;
		}

		void SetRenderObject(RenderObject* newObject) {
			renderObject = newObject;
		}

		void SetPhysicsObject(PhysicsObject* newObject) {
			physicsObject = newObject;
		}

		const std::string& GetName() const {
			return name;
		}

		virtual void OnCollisionBegin(GameObject* otherObject) {
			//std::cout << "OnCollisionBegin event occured!\n";
		}

		virtual void OnCollisionEnd(GameObject* otherObject) {
			//std::cout << "OnCollisionEnd event occured!\n";
		}

		bool GetBroadphaseAABB(Vector3&outsize) const;

		void UpdateBroadphaseAABB();

		void SetWorldID(int newID) {
			worldID = newID;
		}

		int		GetWorldID() const {
			return worldID;
		}

	protected:
		Transform			transform;

		CollisionVolume*	boundingVolume;
		PhysicsObject*		physicsObject;
		RenderObject*		renderObject;
		NetworkObject*		networkObject;

		bool		isCoin;
		bool		isPlayer;
		bool		isActive;
		bool        isTarget;
		bool        isStartArea;
		bool isDoor1a;
		bool isDoor1b;
		bool isobbDoor;
		bool isPortal1;
		bool isPortal2;
		bool isObstacle;
		bool isKey;
		bool isEnemy1;
		bool isGoose;
		bool isDoor1;
		int			worldID;
		std::string	name;

		Vector3 broadphaseAABB;
	};
}

