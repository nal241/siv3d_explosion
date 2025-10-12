#include "SceneGame.h"

namespace
{
    // World settings
    constexpr double WallLength = 10.0;
    constexpr double WallThickness = 1.0;
    constexpr float WallRestitution = 1.0f;

    // Camera settings
    constexpr double CameraSpeed = 20.0;
    constexpr s3d::Vec3 CameraInitialPosition{5, 15, -20};
    constexpr s3d::Vec3 CameraInitialLookAt{5, 0, 10};
    constexpr double CameraFov = 30_deg;

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

SceneGame::SceneGame(const InitData& init)
    : IScene(init), m_renderTexture{Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes}
{

    auto floor = m_world.createBox(BoxDesc{s3d::Vec3(WallLength, WallThickness, WallLength),
                                           s3d::Vec3(WallLength / 2, -WallThickness / 2, WallLength / 2), 0.0f});
    floor->setRestitution(WallRestitution);
    floor->setColor(s3d::Linear::Palette::Silver);

    auto wall_l = m_world.createBox(BoxDesc{s3d::Vec3(WallThickness, WallLength, WallLength),
                                            s3d::Vec3(-WallThickness / 2, WallLength / 2, WallLength / 2), 0.0f});
    wall_l->setRestitution(WallRestitution);
    wall_l->setColor(s3d::Linear::Palette::Powderblue);

    auto wall_r =
        m_world.createBox(BoxDesc{s3d::Vec3(WallThickness, WallLength, WallLength),
                                  s3d::Vec3(WallLength + WallThickness / 2, WallLength / 2, WallLength / 2), 0.0f});
    wall_r->setRestitution(WallRestitution);
    wall_r->setColor(s3d::Linear::Palette::Powderblue);

    auto wall_b =
        m_world.createBox(BoxDesc{s3d::Vec3(WallLength, WallLength, WallThickness),
                                  s3d::Vec3(WallLength / 2, WallLength / 2, WallLength + WallThickness / 2), 0.0f});
    wall_b->setRestitution(WallRestitution);
    wall_b->setColor(s3d::Linear::Palette::Powderblue);

    m_physicsObjects.push_back(std::move(floor));
    m_physicsObjects.push_back(std::move(wall_l));
    m_physicsObjects.push_back(std::move(wall_r));
    m_physicsObjects.push_back(std::move(wall_b));

    m_camera = DebugCamera3D{m_renderTexture.size(), CameraFov, CameraInitialPosition, CameraInitialLookAt};

    for (auto& object : m_physicsObjects)
    {
        object->update();
    }
}

void SceneGame::update()
{
    ClearPrint();
    Print << U"Object num:{}"_fmt(m_physicsObjects.size());
    Print << Profiler::FPS();

    m_camera.update(CameraSpeed);

    for (auto& object : m_physicsObjects)
    {
        object->update();
    }

    // スペースキーでキューブを発射
    if (KeySpace.down())
    {
        // カメラの位置と前方ベクトルを取得
        Vec3 camPos = m_camera.getEyePosition();
        Vec3 camForward = m_camera.getLookAtVector();

        // キューブの初期位置（カメラの少し前）
        Vec3 cubePos = camPos + camForward * 20.0;

        // キューブ生成
        auto shotBox = m_world.createBox(BoxDesc{CubeSize, cubePos, CubeMass});
        shotBox->setRestitution(CubeRestitution);
        shotBox->setColor(s3d::Linear::Palette::Gainsboro);

        // 前方へインパルスを加える
        shotBox->applyImpulse(camForward * CubeLaunchImpulse);
        m_physicsObjects.push_back(std::move(shotBox));

        // キューブ発射音を再生
        // Play()は重複再生しないため、
        // playOneShot()で多重再生する
        m_cubeShootSound.playOneShot();
    }

    // oキーでsphereを発射
    if (KeyO.down())
    {
        // カメラの位置と前方ベクトルを取得
        Vec3 camPos = m_camera.getEyePosition();
        Vec3 camForward = m_camera.getLookAtVector();
        // 球の初期位置（カメラの少し前）
        Vec3 spherePos = camPos + camForward * 20.0;
        // 球生成
        auto shotSphere = m_world.createSphere(SphereDesc{SphereRadius, spherePos, SphereMass});
        shotSphere->setRestitution(SphereRestitution);
        shotSphere->setColor(s3d::Linear::Palette::Lightsteelblue);

        // 前方へインパルスを加える
        shotSphere->applyImpulse(camForward * SphereLaunchImpulse); // 20.0は速度調整
        m_physicsObjects.push_back(std::move(shotSphere));

        // 球発射音を再生
        m_sphereShootSound.playOneShot();
    }

    if (KeyC.down())
    {
        // カメラの位置と前方ベクトルを取得
        Vec3 camPos = m_camera.getEyePosition();
        Vec3 camForward = m_camera.getLookAtVector();
        // 円柱の初期位置（カメラの少し前）
        Vec3 cylinderPos = camPos + camForward * 20.0;
        // モデル付き円柱（コイン）を生成
        auto shotCoin =
            m_world.createModelObject(CylinderDesc{CylinderRadius, CylinderHeight, cylinderPos, CylinderMass}, m_model);
        shotCoin->setRestitution(CylinderRestitution);
        shotCoin->setFriction(0.3f);
        shotCoin->setDamping(0.1f, 0.5f);

        // 前方へインパルスを加える
        shotCoin->applyImpulse(camForward * CylinderLaunchImpulse);
        m_physicsObjects.push_back(std::move(shotCoin));
    }

    // worldのステップを進める
    m_world.step(Scene::DeltaTime());

    // 座標が一定以下ならオブジェクトを削除
    m_physicsObjects.remove_if([](const std::unique_ptr<PhysicsObject>& obj) { return obj->getPosition().y < -10.0; });
}

void SceneGame::draw() const
{
    // Set up a camera in the current 3D scene
    Graphics3D::SetCameraTransform(m_camera);

    // [3D rendering]
    {
        const ScopedRenderTarget3D target{m_renderTexture.clear(m_backgroundColor)};

        // for debug
        // Plane{64}.draw(uvChecker);
        for (auto& object : m_physicsObjects)
        {
            object->draw();
        }
    }

    // [2D rendering]
    {
        // Flush 3D rendering commands before multisample resolve
        Graphics3D::Flush();

        // Multisample resolve
        m_renderTexture.resolve();

        // Transfer renderTexture to the current 2D scene (default scene)
        Shader::LinearToScreen(m_renderTexture);
    }
}