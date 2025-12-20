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
    AddPlane(Vector(0.0f, 0.0f, 1.0f, 0.0f),
             Scene::Material::Lambert(Scene::Color(0.6f, 0.6f, 0.6f)));

    // Top plane
    AddPlane(Vector(height, 0.0f, -1.0f, 0.0f),
             Scene::Material::Lambert(Scene::Color(0.8f, 0.8f, 0.9f)));

    // Left plane
    AddPlane(Vector(halfWidth, 1.0f, 0.0f, 0.0f),
             Scene::Material::Lambert(Scene::Color(0.9f, 0.5f, 0.5f)));

    // Right plane
    AddPlane(Vector(halfWidth, -1.0f, 0.0f, 0.0f),
             Scene::Material::Lambert(Scene::Color(0.5f, 0.9f, 0.5f)));

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
    //   /* const float pheasantX = std::sin(m_pheasantTime)/10.f;
    //    const Motor translation(1.0f, pheasantX * 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    //    pheasant->transform = pheasant->transform * translation;*/
    //  
    //    //tryying to get my bird to look away from the camera
    //    //pheasant->transform = (pheasant->transform * m_cameraFov);

    //    //or
    // /*   BiVector line{ 1,0,1,0,0,0 };
    //    Motor R = Motor::Rotation(m_cameraYaw, line);
    //    pheasant->transform = R * pheasant->transform * ~R;*/
   
    //    //get pos for camera -> in 
        const TriVector origin(0.0f, 0.0f, 0.0f); //safety net 
        m_cameraTarget = (~pheasant->transform * origin * (pheasant->transform)).Grade3();      //TransformPoint with sandwhich + inversed
    }

    //-------------------------------------------------------
    //const TriVector localForward(0, 0, 1);
    //const TriVector localRight(1, 0, 0);
    //float speed{ 10.f };
    //
    //if (auto* pheasant = FindInstance("pheasant"))
    //{
    //    // Camera-relative directions
    //    TriVector camForward =
    //        (m_cameraTarget - m_cameraEye).Normalized();

    //    camForward.e013() = 0.f; // remove vertical motion
    //    camForward.Normalize();

    //    TriVector worldUp(0, 1, 0);
    //    TriVector camRight = camForward.Cross(worldUp);
    //    camRight.Normalize();

    //    // Combine intent
    //    TriVector moveDir =
    //        camForward * m_Intent.forward +
    //        camRight * m_Intent.right;
    //    moveDir.Normalize();

    //    // Convert direction -> ideal line
    //    BiVector moveLine{
    //        moveDir.e032(), // e23
    //        moveDir.e013(), // e31
    //        moveDir.e021(), // e12
    //        0.f, 0.f, 0.f
    //    };

    //    Motor T = Motor::Translation(
    //        speed * deltaTime,
    //        moveLine
    //    );

    //    // WORLD-space translation
    //    pheasant->transform = T * pheasant->transform;
    //}
}

void TestBoxScene::OnInput(const InputState& input) {
    if (input.rightMouseDown) {
        m_cameraYaw += input.mouseDeltaX * m_mouseSensitivity;
        m_cameraPitch += input.mouseDeltaY * m_mouseSensitivity;
        m_cameraPitch = std::clamp(m_cameraPitch, -1.4f, 1.4f);
    }

    m_cameraDistance -= input.scrollDelta * 2.0f;
    m_cameraDistance = std::clamp(m_cameraDistance, 10.0f, 150.0f);

    constexpr float targetY = 15.0f;

    const float camX = std::sin(m_cameraYaw) * std::cos(m_cameraPitch) * m_cameraDistance;
    const float camY = std::sin(m_cameraPitch) * m_cameraDistance + targetY;
    const float camZ = std::cos(m_cameraYaw) * std::cos(m_cameraPitch) * m_cameraDistance;

    m_cameraEye = TriVector(camX, camY, camZ);
    //m_cameraTarget = TriVector(0.0f, targetY, 0.0f);
    m_cameraUp = TriVector(0.0f, 1.0f, 0.0f);

    //direction in game
    //z is forward
    //x pos is left
    //y is upwards

    //reinitialize
    m_Intent.forward = 0.f;
    m_Intent.right = 0.f;

    //handling input for the transformation pheasent
    if (input.keyW || input.keyUp)    m_Intent.forward += 1.f;
    if (input.keyS || input.keyDown)  m_Intent.forward -= 1.f;
    if (input.keyD || input.keyRight) m_Intent.right += 1.f;
    if (input.keyA || input.keyLeft)  m_Intent.right -= 1.f;

    //for direction: join camera and pos bird -> get direction line

    if (auto* pheasant = FindInstance("pheasant"))
    {
        bool translatePheasant{ false };
        const BiVector dirFromCamToMesh{ (m_cameraTarget & m_cameraEye).Normalized()};
        BiVector movementDirection{ 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };


        if (input.keyW || input.keyUp)
        {
            movementDirection.e01() += dirFromCamToMesh.e23();
            movementDirection.e03() += dirFromCamToMesh.e12();
            translatePheasant = true;
        }
        if (input.keyD || input.keyRight)
        {
            movementDirection.e01() -= dirFromCamToMesh.e12();
            movementDirection.e03() += dirFromCamToMesh.e23();
            translatePheasant = true;
        }
        if (input.keyS || input.keyDown)
        {
            movementDirection.e01() -= dirFromCamToMesh.e23();
            movementDirection.e03() += dirFromCamToMesh.e23();
            translatePheasant = true;
        }
        if (input.keyA || input.keyLeft)
        {
            movementDirection.e01() += dirFromCamToMesh.e12();
            movementDirection.e03() -= dirFromCamToMesh.e23();
            translatePheasant = true;
        }


        if (translatePheasant)
        {
            const float movementSpeed{ m_pheasantSpeed / 10.f };
            const Motor T{ Motor::Translation(movementSpeed, movementDirection) };
            pheasant->transform = pheasant->transform * T;
        }

    }
}

void TestBoxScene::OnGui() {
    ImGui::Begin("Test Box Scene");
    ImGui::Text("FPS: %.1f", GetFPS());
    ImGui::Separator();
    ImGui::Text("Box: 100 x 30 x 40");
    ImGui::Text("4 Planes (bottom, top, left, right)");
    ImGui::Text("2 Spheres (rotating)");
    ImGui::Text("1 Pheasant (sine wave, scale=%.2f)", m_pheasantScale);
    ImGui::Text("1 Point Light");
    ImGui::Separator();
    ImGui::Text("Sphere Rotation:");
    ImGui::Text("  Angle: %.2f rad (%.1f deg)", m_rotationAngle, m_rotationAngle * (180.0f / kPi));
    ImGui::SliderFloat("Speed", &m_rotationSpeed, 0.0f, 3.0f);
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

void TestBoxScene::OnShutdown() {}
