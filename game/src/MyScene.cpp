//// game/src/MyScene.cpp
//#include "MyScene.h"
//#include <imgui.h>
//
//MyScene::MyScene(const std::string& resourceDir)
//    : GameScene(resourceDir) {
//}
//
//void MyScene::OnInit(VulkanRenderer* renderer) {
//    // Add scene objects here
//    AddGroundPlane(0.0f, Scene::Material::Lambert(Scene::Color::Gray()));
//    AddSphere(TriVector(0.0f, 2.0f, 0.0f), 2.0f,
//        Scene::Material::PBR(Scene::Color::Red()));
//    AddPointLight(TriVector(5.0f, 10.0f, 5.0f),
//        Scene::Color::White(), 1.5f, 50.0f);
//
//    m_cameraEye = TriVector(0.0f, 5.0f, 15.0f);
//    m_cameraTarget = TriVector(0.0f, 2.0f, 0.0f);
//}
//
//void MyScene::OnUpdate(float deltaTime) {
//    // Animation logic here
//}
//
//void MyScene::OnInput(const InputState& input) {
//    // Input handling here
//}
//
//void MyScene::OnGui() {
//    ImGui::Begin("My Scene");
//    ImGui::Text("Hello, FlyTracer!");
//    ImGui::End();
//}