#pragma once

#include "GameScene.h"
#include <cstdint>
#include <numbers>
#include <vector>

//class MultiVector;
class TestBoxScene final : public GameScene {
public:
    explicit TestBoxScene(const std::string& resourceDir);

    void OnInit(VulkanRenderer* renderer) override;
    void OnUpdate(float deltaTime) override;
    void OnInput(const InputState& input) override;
    void OnGui() override;
    void OnShutdown() override;

private:
    static constexpr float kPi = std::numbers::pi_v<float>;

    // Camera orbit
    float m_cameraYaw{0.0f};
    float m_cameraPitch{0.3f};
    float m_cameraDistance{50.0f};
    float m_mouseSensitivity{0.005f};

    // Sphere rotation
    uint32_t bullet{0};
    float m_sphere1Radius{1.0f};
    float m_sphere1Height{5.0f};

    // Pheasant mesh
    uint32_t m_pheasantMeshId{0};
    float m_pheasantTime{0.0f};
    float m_pheasantSpeed{1.0f};
    float m_pheasantAmplitudeX{20.0f};
    float m_pheasantAmplitudeZ{10.0f};
    float m_pheasantHeight{0.0f};
    float m_pheasantScale{0.5f};

    //Target 
    uint32_t m_Target{ 1 };
    TriVector m_TargetPos{ 0.f, 5.f, 30.f };

    // own variables
    TriVector cameraTargetPlayer;
    float bulletSpeed{};
    const float bulletRadius{ 1.f };
    float m_Timer{ };
    std::vector<TriVector> m_Bullets{};
    bool bulletCalled{ false };
    BiVector bulletDirection{};
    float m_pBulletTimer{};

    enum class walkingstate {
        forwards,
        left,
        right,
        backwards,
        none
    };

    walkingstate m_CurrentWalkingState{ walkingstate::none };
    BiVector m_Direction{};

    struct MoveIntent
    {
        float forward{};
        float right{};
    };
    MoveIntent m_Intent{};


    TriVector RotateDirection(const Motor& M, const TriVector& dir)
    {
        Motor R = { M.s(),   // or data[0]
        0.f, 0.f, 0.f,
        M.e23(),
        M.e31(),
        M.e12(),
        0.f };

        // Sandwich product
        MultiVector mv = R * dir * ~R;

        // Direction lives in trivector part
        return TriVector{
            mv.e032(),
            mv.e013(),
            mv.e021()
        };
    }

    float calcAngleInDegreesBetweenTwoBivecs(const BiVector& line1, const BiVector& line2) const
    {
        const BiVector line1Perp{ 0.f, 0.f, 0.f, -line1.e12(), 0.f, line1.e23() };
        const float cos{ line1 | -line2 };
        const float sin{ line1Perp | -line2 };
        float angleDeg{ std::acos(cos) / 3.14158f * 180.f };
        if (sin > 0.f || angleDeg > 360.f)
            angleDeg = 360.f - angleDeg;
        return angleDeg;
    }

    std::vector<Vector> m_planes;
    std::vector<Vector> m_Planes;

    float movementSpeed{ m_pheasantSpeed / 5.f };

    void CalcRotation();
    bool allowCalcRotation{ false };
    void UpdateBullet(float deltaTime);
    void SpawnBullet();
    void BirdCollisions();
    void CameraCollisions();
};
