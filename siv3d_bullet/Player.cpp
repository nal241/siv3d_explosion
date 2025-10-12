#include "Player.h"

namespace
{
    // Cube settings
    constexpr s3d::Vec3 CubeSize{1.0, 1.0, 1.0};
    constexpr float CubeMass = 5.0f;
    constexpr float CubeRestitution = 0.7f;
    constexpr double CubeLaunchImpulse = 30.0;

    // Sphere settings
    constexpr float SphereRadius = 0.5f;
    constexpr float SphereMass = 5.0f;
    constexpr float SphereRestitution = 0.7f;
    constexpr double SphereLaunchImpulse = 20.0;

    // Cylinder settings
    constexpr float CylinderRadius = 0.5f;
    constexpr float CylinderHeight = 0.2f;
    constexpr float CylinderMass = 1.0f;
    constexpr float CylinderRestitution = 0.1f;
    constexpr double CylinderLaunchImpulse = 20.0;
} // namespace

Player::Player(DebugCamera3D* camera, Model& coinModel) : m_camera(camera), m_coinModel(coinModel) {}

void Player::handleInput(PhysicsWorld& world, Array<std::unique_ptr<PhysicsObject>>& objects)
{ // スペースキーでキューブを発射
    if (KeySpace.down())
    {
        // カメラの位置と前方ベクトルを取得
        assert(m_camera != nullptr && "Camera pointer must not be null. Did you forget to call setCamera?");
        Vec3 camPos = m_camera->getEyePosition();
        Vec3 camForward = m_camera->getLookAtVector();

        // キューブの初期位置（カメラの少し前）
        Vec3 cubePos = camPos + camForward * 20.0;

        // キューブ生成
        auto shotBox = world.createBox(BoxDesc{CubeSize, cubePos, CubeMass});
        shotBox->setRestitution(CubeRestitution);
        shotBox->setColor(s3d::Linear::Palette::Gainsboro);

        // 前方へインパルスを加える
        shotBox->applyImpulse(camForward * CubeLaunchImpulse);
        objects.push_back(std::move(shotBox));

        // キューブ発射音を再生
        // Play()は重複再生しないため、
        // playOneShot()で多重再生する
        m_shootSound.playOneShot();
    }

    // oキーでsphereを発射
    if (KeyO.down())
    {
        // カメラの位置と前方ベクトルを取得
        assert(m_camera != nullptr && "Camera pointer must not be null. Did you forget to call setCamera?");
        Vec3 camPos = m_camera->getEyePosition();
        Vec3 camForward = m_camera->getLookAtVector();
        // 球の初期位置（カメラの少し前）
        Vec3 spherePos = camPos + camForward * 20.0;
        // 球生成
        auto shotSphere = world.createSphere(SphereDesc{SphereRadius, spherePos, SphereMass});
        shotSphere->setRestitution(SphereRestitution);
        shotSphere->setColor(s3d::Linear::Palette::Lightsteelblue);

        // 前方へインパルスを加える
        shotSphere->applyImpulse(camForward * SphereLaunchImpulse); // 20.0は速度調整
        objects.push_back(std::move(shotSphere));

        // 球発射音を再生
        m_shootSound.playOneShot();
    }

    if (KeyC.down())
    {
        // カメラの位置と前方ベクトルを取得
        assert(m_camera != nullptr && "Camera pointer must not be null. Did you forget to call setCamera?");
        Vec3 camPos = m_camera->getEyePosition();
        Vec3 camForward = m_camera->getLookAtVector();
        // 円柱の初期位置（カメラの少し前）
        Vec3 cylinderPos = camPos + camForward * 20.0;
        // モデル付き円柱（コイン）を生成
        auto shotCoin = world.createModelObject(CylinderDesc{CylinderRadius, CylinderHeight, cylinderPos, CylinderMass},
                                                m_coinModel);
        shotCoin->setRestitution(CylinderRestitution);
        shotCoin->setFriction(0.3f);
        shotCoin->setDamping(0.1f, 0.5f);

        // 前方へインパルスを加える
        shotCoin->applyImpulse(camForward * CylinderLaunchImpulse);
        objects.push_back(std::move(shotCoin));
        m_shootSound.playOneShot();
    }
}