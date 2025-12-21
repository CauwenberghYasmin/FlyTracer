#include "TestBoxScene.h"
#include <algorithm>
#include <cmath>
#include <imgui.h>
#include <iostream>

TestBoxScene::TestBoxScene(const std::string& resourceDir)
    : GameScene(resourceDir) {}

void TestBoxScene::OnInit([[maybe_unused]] VulkanRenderer* renderer) {
    constexpr float halfWidth = 50.0f;
    constexpr float height = 30.0f;

    // Bottom plane
    AddPlane (Vector(0.0f, 0.0f, 1.0f, 0.0f),
             Scene::Material::Lambert(Scene::Color(0.6f, 0.6f, 0.6f)));

    // Top plane
    AddPlane(Vector(height, 0.0f, -1.0f, 0.0f),
             Scene::Material::Lambert(Scene::Color(0.8f, 0.8f, 0.9f)));

    // Left plane blue
    AddPlane(Vector(halfWidth, 1.0f, 0.0f, 0.0f),
             Scene::Material::Lambert(Scene::Color(0.9f, 0.5f, 0.5f)));

    // Right plane
    AddPlane(Vector(halfWidth, -1.0f, 0.0f, 0.0f),
             Scene::Material::Lambert(Scene::Color(0.5f, 0.9f, 0.5f)));

    //push back all planes in vector
    m_planes.push_back(std::move(Vector(-height/2, 0.0f, 1.0f, 0.0f)));//top
    m_planes.push_back(std::move(Vector(-halfWidth / 2, -1.0f, 0.0f, 0.0f)));//left
    m_planes.push_back(std::move(Vector(-halfWidth / 2, 1.0f, 0.0f, 0.0f)));//right
    m_planes.push_back(std::move(Vector(0.0f, 0.0f, -1.0f, 0.0f))); //bootom
    m_Planes.push_back(std::move(Vector(-halfWidth, -1.0f, 0.0f, 0.0f)));
    m_Planes.push_back(std::move(Vector(-halfWidth, 1.0f, 0.0f, 0.0f)));


    //+ make normals point inwards


    // Pheasant mesh
    m_pheasantMeshId = LoadMesh("pheasant.obj", "pheasant.png");
    AddMeshInstance(m_pheasantMeshId, TriVector(0.0f, m_pheasantHeight, 0.0f), "pheasant");//get's loaded center screen

    if (auto* pheasant = FindInstance("pheasant")) {
        pheasant->scale = m_pheasantScale;
        m_cameraTarget = { pheasant->transform.e01()*2, pheasant->transform.e02()*2, pheasant->transform.e03()*2 };
    }

    // Point light
    AddPointLight(TriVector(0.0f, 20.0f, 0.0f),
                  Scene::Color(1.0f, 1.0f, 1.0f), 2.0f, 100.0f);

    AddPointLight(TriVector(0.0f, 100.0f, 0.0f),
        Scene::Color(1.0f, 1.0f, 1.0f), 2.0f, 100.0f);

    // Camera
    m_cameraEye = TriVector(0.0f, 15.0f, 60.0f);
    m_cameraUp = TriVector(0.0f, 1.0f, 0.0f, 0.0f);
}

void TestBoxScene::OnUpdate(float deltaTime) {
    UpdateFPS(deltaTime);

    //Update pheasant
    m_pheasantTime += m_pheasantSpeed * deltaTime;


    if (auto* pheasant = FindInstance("pheasant")) {
        //get pos for camera -> in 
        const TriVector origin(0.0f, 0.0f, 0.0f); //safety net 
        m_cameraTarget = (~pheasant->transform * origin * (pheasant->transform)).Grade3();      //bird always center screen  

        //------------------------collision walls----------------------------

        TriVector currentPos = m_cameraTarget;

        for (size_t index = 0; index < m_Planes.size(); ++index) 
        {
            currentPos /= currentPos.e123();

            float distance = (m_Planes[index] ^ currentPos).e0123();
            float meshRadius = 20.f * pheasant->scale; // size bird
       
            if (distance < meshRadius) {
                std::cout << "collided\n";

                float margin{ 0.5f };
                float pushAmount = (meshRadius + margin) - distance;

                currentPos.e032() -= m_Planes[index].e1() * pushAmount;
                currentPos.e013() -= m_Planes[index].e2() * pushAmount;
                currentPos.e021() -= m_Planes[index].e3() * pushAmount;

                SetInstancePosition(m_pheasantMeshId, currentPos);

            }
        }        
    }

    //-----------------check collision with camera---------------------
    for (size_t index = 0; index < m_planes.size(); ++index)
    {
        TriVector cameraPoint = m_cameraEye;
        cameraPoint /= cameraPoint.e123(); //weight e123 = 1

        MultiVector wedgeResult = m_planes[index] ^ cameraPoint; // calc distance
        float signedDistance = wedgeResult.e0123();

        float cameraRadius = 0.03f; //safety net or boundry

        if (signedDistance < cameraRadius)
        {
            m_cameraEye = (m_planes[index] * m_cameraEye * ~m_planes[index]).Grade3();
        }
    }


    if (m_cameraEye.e013() == NAN) //small safety measure
    {
        std::cout << "camera crashed";
        m_cameraEye = TriVector(0.0f, 15.0f, 60.0f);
    }
}   

void TestBoxScene::OnInput(const InputState& input) {
    if (input.rightMouseDown) {
        m_cameraYaw -= input.mouseDeltaX * m_mouseSensitivity;
        m_cameraPitch += input.mouseDeltaY * m_mouseSensitivity;
        m_cameraPitch = std::clamp(m_cameraPitch, -1.55f, 1.55f);


        //----------ROTATION----------------            //only when redirecting camera (right mouse button)
        BiVector desiredForward = (m_cameraTarget & m_cameraEye).Normalized(); //opposite, because bird should face away!
        desiredForward.e31() = 0; // can't go up
        desiredForward = desiredForward.Normalized();
        const BiVector localForward{ 0,0,0,0,0,1 };

        if (auto* pheasant = FindInstance("pheasant")) {
            BiVector currentForward = (pheasant->transform * -localForward * ~pheasant->transform).Grade2();
            currentForward = currentForward.Normalized();
            float angle = acos(std::clamp(- currentForward | desiredForward, -1.f, 1.f)); //only works with normalized!!!

            float curX = currentForward.e23();
            float curZ = currentForward.e12();
            float desX = desiredForward.e23();
            float desZ = desiredForward.e12();

            // 2D Cross product (determinant) tells us if Desired is Left or Right of Current
            float side = (curX * desZ) - (curZ * desX); //(ai gave the idea, not the code!)
            float sign = (side < 0) ? 1.0f : -1.0f; 

            if (angle > 0.05f)
            {
                BiVector rotationLine(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
                float rotationStep{ 10.f };
                Motor R = Motor::Rotation((angle * rotationStep) * sign, rotationLine);
                pheasant->transform = R * pheasant->transform;
            }
        }
    }

    m_cameraDistance -= input.scrollDelta * 2.0f;
    m_cameraDistance = std::clamp(m_cameraDistance, 10.0f, 150.0f);

    constexpr float targetY = 15.0f;

    const float camX = std::sin(m_cameraYaw) * std::cos(m_cameraPitch) * m_cameraDistance;
    const float camY = std::sin(m_cameraPitch) * m_cameraDistance + targetY;
    const float camZ = std::cos(m_cameraYaw) * std::cos(m_cameraPitch) * m_cameraDistance;

    m_cameraUp = TriVector(0.0f, 1.0f, 0.0f);

    // nan check
    if (!std::isnan(camX) && !std::isnan(camY) && !std::isnan(camZ)) {
        m_cameraEye = m_cameraTarget + TriVector(camX, camY, camZ); //cam follows bird
    }
 
    //direction in game
    //z is forward
    //x pos is left
    //y is upwards

    //------------------movement-----------------------------

    if (auto* pheasant = FindInstance("pheasant"))
    {
        bool translatePheasant{ false };
        const BiVector camDirection{ (m_cameraTarget & m_cameraEye).Normalized() };
        BiVector movementDirection{ 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
        if (input.keyW)
        {
            movementDirection.e03() += camDirection.e12();
            movementDirection.e01() += camDirection.e23();
            translatePheasant = true;
        }
        if (input.keyA)
        {
            movementDirection.e03() -= camDirection.e23();
            movementDirection.e01() += camDirection.e12();
            translatePheasant = true;
        }
        if (input.keyS)
        {
            movementDirection.e03() -= camDirection.e12();
            movementDirection.e01() -= camDirection.e23();
            translatePheasant = true;
        }
        if (input.keyD)
        {
            movementDirection.e03() += camDirection.e23();
            movementDirection.e01() -= camDirection.e12();
            translatePheasant = true;
        }
        if (translatePheasant) //only do if buttons pressed
        {
            const Motor T{ Motor::Translation(movementSpeed, movementDirection) };
            pheasant->transform = pheasant->transform * T;
        }
    }


    if (input.key1)
    {
        std::cout << "----------------midpoint---------------\n"; //for testing
    }
}

void TestBoxScene::OnGui() {
    ImGui::Begin("Test Box Scene");
    ImGui::Text("FPS: %.1f", GetFPS());
    ImGui::Separator();
    ImGui::SliderFloat("Speed pheasant", &movementSpeed, 0.0f, 2.0f);
    ImGui::Separator();
    ImGui::Text("Controls:");
    ImGui::Text("  Right-drag: Orbit camera");
    ImGui::Text("  Scroll: Zoom");
    ImGui::Separator();
    ImGui::Text("Camera Yaw: %.2f", m_cameraYaw);
    ImGui::Text("Camera Pitch: %.2f", m_cameraPitch);
    ImGui::Text("Camera Distance: %.1f", m_cameraDistance);
    ImGui::End();
}

void TestBoxScene::OnShutdown() {
    m_planes.clear();
}
