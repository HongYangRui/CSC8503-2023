#include "StateGameObject.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "PhysicsObject.h"

using namespace NCL;
using namespace CSC8503;

StateGameObject::StateGameObject() {
	counter = 0.0f;
	stateMachine = new StateMachine();

	State* stateA = new State([&](float dt)->void {
		this->MoveLeft(dt);
		});

	State* stateB = new State([&](float dt)->void {
		this->MoveRight(dt);
		});

	State* stateC = new State([&](float dt)->void {
		this->MoveForward(dt);
		});

	State* stateD = new State([&](float dt)->void {
		this->MoveBackward(dt);
		});

	/*stateMachine->AddState(stateA);
	stateMachine->AddState(stateB);*/
	stateMachine->AddState(stateC);
	stateMachine->AddState(stateD);

	/*stateMachine->AddTransition(new StateTransition(stateA, stateB, [&]()->bool{
		return this->counter > 3.0f;
		}));
	stateMachine->AddTransition(new StateTransition(stateB, stateA, [&]()->bool {
		return this->counter < 0.0f;
		}));*/

	stateMachine->AddTransition(new StateTransition(stateC, stateD, [&]()->bool {
		return this->counter > 5.0f;
		}));
	stateMachine->AddTransition(new StateTransition(stateD, stateC, [&]()->bool {
		return this->counter < -8.0f;
		}));

}

StateGameObject::~StateGameObject() {
	delete stateMachine;
}

void StateGameObject::Update(float dt) {
	stateMachine->Update(dt);
}

void StateGameObject::MoveLeft(float dt) {
	GetPhysicsObject()->AddForce({ -100,0,0 });
	counter += dt;
}

void StateGameObject::MoveRight(float dt) {
	GetPhysicsObject()->AddForce({ 100,0,0 });
	counter -= dt;
}

void StateGameObject::MoveForward(float dt) {
	GetTransform().SetOrientation(Quaternion(Matrix4::Rotation(360, Vector3(0, 1, 0))));
	GetPhysicsObject()->AddForce({ 0,0,-10 });
	counter += dt;
}

void StateGameObject::MoveBackward(float dt) {
	GetTransform().SetOrientation(Quaternion(Matrix4::Rotation(180, Vector3(0, 1, 0))));
	GetPhysicsObject()->AddForce({ 0,0,10 });
	counter -= dt;
}

