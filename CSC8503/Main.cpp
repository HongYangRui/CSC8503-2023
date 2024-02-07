#include "Window.h"

#include "Debug.h"

#include "StateMachine.h"
#include "StateTransition.h"
#include "State.h"

#include "GameServer.h"
#include "GameClient.h"

#include "NavigationGrid.h"
#include "NavigationMesh.h"

#include "TutorialGame.h"
#include "NetworkedGame.h"

#include "PushdownMachine.h"

#include "PushdownState.h"

#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"

//#include "LevelOne.h"
//#include "MainMenu.h"

using namespace NCL;
using namespace CSC8503;

#include <chrono>
#include <thread>
#include <sstream>
//void TestBehaviourTree() {
//	float behaviourTimer;
//	float distanceTotarget;
//	BehaviourAction* findKey = new BehaviourAction("Find Key", [&](float dt, BehaviourState state)->BehaviourState {
//		if (state == Initialise) {
//			std::cout << "Looking for a key!\n";
//			behaviourTimer = rand() % 100;
//			state = Ongoing;
//		}
//		else if (state == Ongoing) {
//			behaviourTimer -= dt;
//			if (behaviourTimer <= 0.0f) {
//				std::cout << "Found a key!\n";
//				return Success;
//			}
//		}
//		return state;//will be ongoing until success
//		});
//	BehaviourAction* goToRoom = new BehaviourAction("Go To Room", [&](float dt, BehaviourState state)->BehaviourState {
//		if (state == Initialise) {
//			std::cout << "Going to the loot room!\n";
//			state = Ongoing;
//		}
//		else if (state == Ongoing) {
//			distanceTotarget -= dt;
//			if (distanceTotarget <= 0.0f) {
//				std::cout << "Going to the loot room!\n";
//				return Success;
//			}
//		}
//		return state;//will be ongoing until success
//		});
//	BehaviourAction* openDoor = new BehaviourAction("Open Door", [&](float dt, BehaviourState state)->BehaviourState {
//		if (state == Initialise) {
//			std::cout << "Opening Door!\n";
//			return Success;
//		}
//		return state;
//		});
//	BehaviourAction* lookForTreasure = new BehaviourAction("Look For Treasure", [&](float dt, BehaviourState state)->BehaviourState {
//		if (state == Initialise) {
//			std::cout << "Looking for Treasure!\n";
//			return Ongoing;
//		}
//		else if (state == Ongoing) {
//			bool found = rand() % 2;
//			if (found) {
//				std::cout << "I found some treasure!\n";
//				return Success;
//			}
//			std::cout << "No treasure in here...\n";
//			return Failure;
//		}
//		return state;
//		});
//	BehaviourAction* lookForItems = new BehaviourAction("Look For Items", [&](float dt, BehaviourState state)->BehaviourState {
//		if (state == Initialise) {
//			std::cout << "Looking for items!\n";
//			return Ongoing;
//		}
//		else if (state == Ongoing) {
//			bool found = rand() % 2;
//			if (found) {
//				std::cout << "I found some items!\n";
//				return Success;
//			}
//			std::cout << "No items in here...\n";
//			return Failure;
//		}
//		return state;
//		});
//	BehaviourSequence* sequence = new BehaviourSequence("Room Sequence");
//	sequence->AddChild(findKey);
//	sequence->AddChild(goToRoom);
//	sequence->AddChild(openDoor);
//	BehaviourSelector* selection = new BehaviourSelector("Loot Selection");
//	selection->AddChild(lookForTreasure);
//	selection->AddChild(lookForItems);
//	BehaviourSequence* rootSequence = new BehaviourSequence("Root Sequence");
//	rootSequence->AddChild(sequence);
//	rootSequence->AddChild(sequence);
//	for (int i = 0; i < 5; ++i) {
//		rootSequence->Reset();
//		behaviourTimer = 0.0f;
//		distanceTotarget = rand() % 250;
//		BehaviourState state = Ongoing;
//		std::cout << "We are going on an adventure!\n";
//		while (state == Ongoing) {
//			state = rootSequence->Execute(1.0f);//fake dt
//		}
//		if (state == Failure) {
//			std::cout << "What a waste of time!\n";
//		}
//	}
//	std::cout << "All done!\n";
//}
//void TestPathfinding() {
//	NavigationGrid grid("TestGrid1.txt");
//	NavigationPath outPath;
//	Vector3 startPos(80, 0, 10);
//	Vector3 endPos(80, 0, 80);
//	bool found = grid.FindPath(startPos, endPos, outPath);
//	Vector3 pos;
//	while (outPath.PopWaypoint(pos)) {
//		testNodes.push_back(pos);
//	}
//}a



void TestStateMachine() {
	StateMachine* testMachine = new StateMachine();
	int data = 0;
	State* A = new State([&](float dt)->void {
		std::cout << "I'm in state A!\n";
		data++;
		}
	);
	State* B = new State([&](float dt)->void {
		std::cout << "I'm in state B!\n";
		data--;
		}
	);
	StateTransition* stateAB = new StateTransition(A, B, [&](void)->bool {
		return data > 10;
		}
	);
	StateTransition* stateBA = new StateTransition(B, A, [&](void)->bool {
		return data < 0;
		}
	);
	testMachine->AddState(A);
	testMachine->AddState(B);
	testMachine->AddTransition(stateAB);
	testMachine->AddTransition(stateBA);
	for (int i = 0; i < 100; i++) {
		testMachine->Update(1.0f);
	}

}

class PauseScreen : public PushdownState {
	PushdownResult OnUpdate(float dt, PushdownState** newState)override {
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::U)) {
			return PushdownResult::Pop;
		}
		return PushdownResult::NoChange;
	}
	void OnAwake() override {
		std::cout << "Press U to unpause game!\n";
	}
};

class GameScreen : public PushdownState {
	 PushdownResult OnUpdate(float dt,
		 PushdownState * *newState) override {
		 pauseReminder -= dt;
		 if (pauseReminder < 0) {
			 std::cout << " Coins mined : " << coinsMined << "\n";
			 std::cout << " Press P to pause game , or F1 to return to main menu !\n";
				 pauseReminder += 1.0f;
			
		}
		 if (Window::GetKeyboard() -> KeyDown(KeyCodes::P)) {
			 * newState = new PauseScreen();
			 return PushdownResult::Push;
			
		}
		 if (Window::GetKeyboard() -> KeyDown(KeyCodes::F1)) {
			 std::cout << " Returning to main menu !\n";
			 return PushdownResult::Pop;
			
		}
		 if (rand() % 7 == 0) {
			 coinsMined++;
			
		}
		 return PushdownResult::NoChange;
		
	};
	 void OnAwake() override {
		 std::cout << " Preparing to mine coins !\n";
		
	}
 protected:
	 int coinsMined = 0;
	 float pauseReminder = 1;
};


class IntroScreen :public PushdownState {
	PushdownResult OnUpdate(float dt, PushdownState** newState)override {
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
			*newState = new GameScreen();
			return PushdownResult::Push;
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
			return PushdownResult::Pop;
		}
		return PushdownResult::NoChange;
	};
	void OnAwake()override {
		std::cout << "Welcome to a really awesome game!\n";
		std::cout << "Press Space To Begin or escape to quit!\n";
	}
};

void TestPushdownAutomata(Window* w,TutorialGame* g) {
	PushdownMachine machine(new IntroScreen());
	while (w->UpdateWindow()) {
		float dt = w->GetTimer().GetTimeDeltaSeconds();
		if (!machine.Update(dt)) {
			return;
		}
		g->UpdateGame(dt);
	}
}

class TestPacketReceiver :public PacketReceiver {
public:
	TestPacketReceiver(std::string name) {
		this->name = name;
	}
	void ReceivePacket(int type, GamePacket* payload, int source) {
		if (type == String_Message) {
			StringPacket* realPacket = (StringPacket*)payload;
			std::string msg = realPacket->GetStringFromData();
			std::cout << name << "received message:" << msg << std::endl;
		}
	}
protected:
	std::string name;
};
void TestNetworking() {
	NetworkBase::Initialise();
	TestPacketReceiver serverReceiver("Server");
	TestPacketReceiver clientReceiver1("Client1");
	TestPacketReceiver clientReceiver2("Client2");
	int port = NetworkBase::GetDefaultPort();
	GameServer* server = new GameServer(port, 2);
	GameClient* client1 = new GameClient();
	GameClient* client2 = new GameClient();
	
	server->RegisterPacketHandler(String_Message, &serverReceiver);
	client1->RegisterPacketHandler(String_Message, &clientReceiver1);
	client2->RegisterPacketHandler(String_Message, &clientReceiver2);
	bool canConnect1 = client1->Connect(127, 0, 0, 1, port);
	bool canConnect2 = client2->Connect(127, 0, 0, 1, port);
	

	for (int i = 0; i < 100; ++i) {
		//server->SendGlobalPacket(StringPacket("Server says hello!" + std::to_string(i)));
		server->SendGlobalPacket((GamePacket&)StringPacket("Server says hello!" + std::to_string(i)));
		/*client->SendPacket(StringPacket("Client says hello!" + std::to_string(i)));*/
		client1->SendPacket((GamePacket&)StringPacket("Client1 says hello!" + std::to_string(i)));
		
		client2->SendPacket((GamePacket&)StringPacket("Client2 says hello!" + std::to_string(i)));

		server->UpdateServer();
		client1->UpdateClient();
		client2->UpdateClient();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	NetworkBase::Destroy();
}
/*

The main function should look pretty familar to you!
We make a window, and then go into a while loop that repeatedly
runs our 'game' until we press escape. Instead of making a 'renderer'
and updating it, we instead make a whole game, and repeatedly update that,
instead. 

This time, we've added some extra functionality to the window class - we can
hide or show the 

*/
int main() {
	Window*w = Window::CreateGameWindow("CSC8503 Game technology!", 960, 1200,false);
	if (!w->HasInitialised()) {
		return -1;
	}	

	w->ShowOSPointer(false);
	w->LockMouseToWindow(true);
	TestNetworking();
	TutorialGame* g = new TutorialGame();
	
	w->GetTimer().GetTimeDeltaSeconds(); //Clear the timer so we don't get a larget first dt!
	/*TestPathfinding();*/
	
	
	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyCodes::ESCAPE)) {
		float dt = w->GetTimer().GetTimeDeltaSeconds();
		/*TestPushdownAutomata(w,g);*/
		/*TestBehaviourTree();*/
		if (dt > 0.1f) {
			std::cout << "Skipping large time delta" << std::endl;
			continue; //must have hit a breakpoint or something to have a 1 second frame time!
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::PRIOR)) {
			w->ShowConsole(true);
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::NEXT)) {
			w->ShowConsole(false);
		}

		/*if (Window::GetKeyboard()->KeyPressed(KeyCodes::T)) {
			w->SetWindowPosition(0, 0);
		}*/

		w->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));

		g->UpdateGame(dt);
	}
	Window::DestroyGameWindow();
}