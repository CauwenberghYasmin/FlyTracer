#include "TestBoxScene.h"
#include <algorithm>
#include <cmath>
#include <imgui.h>
#include <iostream>

TestBoxScene::TestBoxScene(const std::string& resourceDir)
    : GameScene(resourceDir) {}

void TestBoxScene::OnInit([[maybe_unused]] VulkanRenderer* renderer) {
    constexpr float halfWidth = 50.0f;
    constexpr float height = 50.0f;

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
    m_planes.push_back(std::move(Vector(0.0f, 0.0f, -1.0f, 0.0f))); //bottom
    m_Planes.push_back(std::move(Vector(-halfWidth, -1.0f, 0.0f, 0.0f)));
    m_Planes.push_back(std::move(Vector(-halfWidth, 1.0f, 0.0f, 0.0f)));
    m_Planes.push_back(std::move(Vector(0.0f, 0.0f, -1.0f, 0.0f)));
    m_Planes.push_back(std::move(Vector(-height, 0.0f, 1.0f, 0.0f)));
    //+ make normals point inwards


    // Pheasant mesh
    m_pheasantMeshId = LoadMesh("pheasant.obj", "pheasant.png");
    AddMeshInstance(m_pheasantMeshId, TriVector(0.0f, m_pheasantHeight, 0.0f), "pheasant");//get's loaded center screen

    if (auto* pheasant = FindInstance("pheasant")) {
        pheasant->scale = m_pheasantScale;
        m_cameraTarget = { pheasant->transform.e01()*2, pheasant->transform.e02()*2, pheasant->transform.e03()*2 };
    }

    //mesh target
    m_Target = LoadMesh("target.obj", "target.jpg");
    AddMeshInstance(m_Target, m_TargetPos, "Target");

    if (auto* target = FindInstance("Target")) {
        target->scale = 0.1;
    }

    // Point light
    AddPointLight(TriVector(0.0f, 20.0f, 0.0f),
                  Scene::Color(1.0f, 1.0f, 1.0f), 2.0f, 100.0f);

    AddPointLight(TriVector(0.0f, 100.0f, 0.0f),
        Scene::Color(1.0f, 1.0f, 1.0f), 2.0f, 100.0f);

    // Camera
    m_cameraEye = TriVector(0.0f, 15.0f, 60.0f);
    m_cameraUp = TriVector(0.0f, 1.0f, 0.0f, 0.0f);

    //---------can only initialize meshes here!-------------
    bullet = AddSphere(TriVector(-5,-5,-5), bulletRadius,      //putting ball out of sight
        Scene::Material::Metal(Scene::Color(0.9f, 0.3f, 0.3f), 0.2f));
}

void TestBoxScene::OnUpdate(float deltaTime) {
    UpdateFPS(deltaTime);
    m_Timer += deltaTime;
    m_pBulletTimer += deltaTime;
    //CalcRotation();

    //Update pheasant
    m_pheasantTime += m_pheasantSpeed * deltaTime;

    BirdCollisions();
    CameraCollisions();

    if (bulletCalled) //function to update bullet (only if spawned)
        UpdateBullet(deltaTime);
    
}   



void TestBoxScene::OnInput(const InputState& input) {
    if (input.rightMouseDown) {
        m_cameraYaw -= input.mouseDeltaX * m_mouseSensitivity;
        m_cameraPitch += input.mouseDeltaY * m_mouseSensitivity;
        m_cameraPitch = std::clamp(m_cameraPitch, -1.55f, 1.55f);

        if (allowCalcRotation)
        {
            CalcRotation(); //only when pressing mouse!
        }
    }
    if (input.keyQ)
    {
        allowCalcRotation = true;
    }

    m_cameraDistance -= input.scrollDelta * 2.0f;
    m_cameraDistance = std::clamp(m_cameraDistance, 10.0f, 150.0f);

    constexpr float targetY = 15.0f;

    const float camX = std::sin(m_cameraYaw) * std::cos(m_cameraPitch) * m_cameraDistance;
    const float camY = std::sin(m_cameraPitch) * m_cameraDistance + targetY;
    const float camZ = std::cos(m_cameraYaw) * std::cos(m_cameraPitch) * m_cameraDistance;

    m_cameraUp = TriVector(0.0f, 1.0f, 0.0f);
    m_cameraEye = m_cameraTarget + TriVector(camX, camY, camZ); //cam follows bird 

    //---------------Movement bird------------------------------
    if (auto* pheasant = FindInstance("pheasant"))
    {
        const BiVector lookLine = (m_cameraTarget & m_cameraEye).Normalized();
        BiVector forward(lookLine.e23(), 0, lookLine.e12(), 0, 0, 0);

        // rotating "Forward" 90 degrees
        static const Motor rotate90 = Motor::Rotation(90, BiVector(0, 0, 0, 0, 1, 0));
        BiVector right = (rotate90 * forward * ~rotate90).Grade2();

        BiVector moveDir{ 0,0,0,0,0,0 };
        bool isMoving = false;

        //+= for diagonal movement (press keys same time)
        if (input.keyW) { moveDir += forward; isMoving = true; }
        if (input.keyS) { moveDir -= forward; isMoving = true; }
        if (input.keyD) { moveDir -= right;   isMoving = true; }
        if (input.keyA) { moveDir += right;   isMoving = true; }

        if (isMoving)
        {
            float norm = moveDir.VNorm();
            if (norm > 0.0001f)
            {
                const Motor T = Motor::Translation(movementSpeed, moveDir);
                pheasant->transform = pheasant->transform * T;
            }
        }
    }

    //--------------------gun shooting------------------
    if (input.keyShift && m_Timer > 1.f) //makes sure no spam pressing
        SpawnBullet();
}

void TestBoxScene::OnGui() {
    ImGui::Begin("Test Box Scene");
    ImGui::Text("FPS: %.1f", GetFPS());
    ImGui::Text("PRESS Q for rotations!");
    ImGui::Separator();
    ImGui::SliderFloat("Speed pheasant", &movementSpeed, 0.0f, 2.0f);
    ImGui::Separator();
    ImGui::SliderFloat("Speed bullets", &bulletSpeed, 0.0f, 10.0f);
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
    m_Planes.clear();
}

void TestBoxScene::CalcRotation()
{
    BiVector desiredForward = (m_cameraTarget & m_cameraEye).Normalized(); //opposite, because bird should face away!
    desiredForward.e31() = 0; // can't go up
    desiredForward = desiredForward.Normalized();
    const BiVector localForward{ 0,0,0,0,0,1 };

    if (auto* pheasant = FindInstance("pheasant")) {
        BiVector currentForward = (pheasant->transform * -localForward * ~pheasant->transform).Grade2();
        currentForward = currentForward.Normalized();
        float angle = acos(std::clamp(-currentForward | desiredForward, -1.f, 1.f)); //only works with normalized!!!

        float curX = currentForward.e23();
        float curZ = currentForward.e12();
        float desX = desiredForward.e23();
        float desZ = desiredForward.e12();

        // 2D Cross product (determinant) tells us if Desired is Left or Right of Current
        float side = (curX * desZ) - (curZ * desX); //(ai)
        float sign = (side < 0) ? 1.0f : -1.0f;
        //--
        if (angle > 0.05f)
        {
            BiVector rotationLine(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
            float rotationStep{ 10.f };
            Motor R = Motor::Rotation((angle * rotationStep) * sign, rotationLine);
            pheasant->transform = R * pheasant->transform;
        }
    }
}

void TestBoxScene::SpawnBullet()
{
    std::cout << "pressed shooting";
    m_Timer = 0.f;
    bulletSpeed = 70.f;
    float frontOffset = 10.0f;
    bulletCalled = true;

    //bullet spawn above ground
    TriVector spawnPos{ m_cameraTarget }; //pos bird

    //bullet spawned in front of pheasant
    bulletDirection = (m_cameraTarget & m_cameraEye).Normalized(); //get's direction line when spawned, doesn't change during the update function!
    Motor moveForward = Motor::Translation(frontOffset, -!bulletDirection);//sign makes sure bullet spawns in front
    spawnPos = (moveForward * spawnPos * ~moveForward).Grade3();
    //clamping so it spawns above the floor
    spawnPos.e013() = std::clamp(spawnPos.e013(), m_cameraTarget.e013() + (bulletRadius * 2), 100.f);

    Scene::GPUSphere* sphere = GetSphere(bullet);
    if (sphere)
    {
        sphere->SetCenter(spawnPos);
    }
}

void TestBoxScene::UpdateBullet(float deltaTime)
{
    Scene::GPUSphere* sphere = GetSphere(bullet);
    if (!sphere) return;

    // 1. Friction & Speed Management
    if (bulletSpeed > 0.01f) {
        float friction = 0.98f;
        bulletSpeed *= std::pow(friction, deltaTime * 40.0f); //speed slows down
    }
    else {
        bulletSpeed = 0.0f; // Completely stop if too slow
    }

    // gravity
    TriVector currentCenter(sphere->center[0], sphere->center[1], sphere->center[2], 1.0f);
    bool onGround = (currentCenter.e013() <= bulletRadius + 0.05f);

    if (!onGround || bulletSpeed > 0.1f) {
        float gravityStrength = 2.0f * deltaTime;
        bulletDirection.e31() += gravityStrength;
        bulletDirection = bulletDirection.Normalized();
    }

    // movement
    Motor T = Motor::Translation(bulletSpeed * deltaTime, -!bulletDirection); //set's direction ball
    TriVector nextPos = (T* currentCenter * ~T).Grade3();

    // 4. Collision Loop
    for (size_t i = 0; i < m_Planes.size(); ++i)
    {
        float distance = (m_Planes[i] ^ nextPos).e0123();

        if (std::abs(distance) < bulletRadius)
            bulletDirection = { 0,0,0,0,0,0 }; //ball stops and will only go down due to gravity
    }

    if (auto* target = FindInstance("Target")) {
        TriVector diff = nextPos - m_TargetPos;

        // Calc distance 
        float distSq = diff.e032() * diff.e032() + diff.e013() * diff.e013() + diff.e021() * diff.e021();
        float hitRadius = bulletRadius + 5.0f; // hardcoded target size 

        if (distSq < (hitRadius * hitRadius)) {
            std::cout << "TARGET HIT!" << std::endl;
            bulletSpeed = 0.0f;
            bulletDirection = { 0,0,0,0,0,0 };
        }
    }
    
    //can't go below floor
    if (nextPos.e013() < bulletRadius) {
        nextPos.e013() = bulletRadius;
    }

    sphere->SetCenter(nextPos);
}

void TestBoxScene::CameraCollisions()
{
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
}

void TestBoxScene::BirdCollisions()
{
    if (auto* pheasant = FindInstance("pheasant")) {
        //get pos for camera -> in 
        const TriVector origin(0.0f, 0.0f, 0.0f); //safety net 
        m_cameraTarget = (~pheasant->transform * origin * (pheasant->transform)).Grade3();      //bird always center screen  
        TriVector currentPos = m_cameraTarget;

        for (size_t index = 0; index < m_Planes.size()-2; ++index)
        {
            currentPos /= currentPos.e123();

            float distance = (m_Planes[index] ^ currentPos).e0123();
            float meshRadius = 20.f * pheasant->scale; // size bird

            if (distance < meshRadius) 
            {
                float margin{ 0.5f };
                float pushAmount = (meshRadius + margin) - distance;
                currentPos.e032() -= m_Planes[index].e1() * pushAmount;
                currentPos.e013() -= m_Planes[index].e2() * pushAmount;
                currentPos.e021() -= m_Planes[index].e3() * pushAmount;
                SetInstancePosition(m_pheasantMeshId, currentPos);

                CalcRotation();
            }
        }
    }
}