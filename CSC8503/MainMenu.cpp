//#include "MainMenu.h"
//
//using namespace NCL;
//using namespace CSC8503;
//MainMenu::MainMenu() :TutorialGame() {
//	InitCamera();
//	InitWorld();
//}
//void MainMenu::InitWorld() {
//	world->ClearAndErase();
//	physics->Clear();
//}
//void MainMenu::UpdateGame(float dt) {
//	Debug::Print("220134488", Vector2(30, 30),Vector4(1,0,0,1));
//	Debug::Print("YANGRUI HONG", Vector2(30, 40),Vector4(0,1,0,1));
//	Debug::Print("Press 1 to level 1", Vector2(30, 50),Vector4(0, 0, 1, 1));
//	Debug::Print("Press 2 to level 2", Vector2(30, 60),Vector4(0, 1, 0, 1));
//	Debug::Print("Press ESC to exit", Vector2(30, 70), Vector4(0, 1, 1, 1));
//	world->UpdateWorld(dt);
//	renderer->Update(dt);
//	Debug::UpdateRenderables(dt);
//	renderer->Render();
//}