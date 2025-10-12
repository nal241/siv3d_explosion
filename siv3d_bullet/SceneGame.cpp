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
} // namespace

SceneGame::SceneGame(const InitData& init)
    : IScene(init), m_renderTexture{Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes},
      m_player(&m_camera, m_model)
{
    // stage作成
    createStage();

    // カメラ設定
    m_camera = DebugCamera3D{m_renderTexture.size(), CameraFov, CameraInitialPosition, CameraInitialLookAt};
}

void SceneGame::update()
{
    ClearPrint();
    Print << U"Object num:{}"_fmt(m_physicsObjects.size());
    Print << U"Tキーで爆発用のシーンへ移動";
    Print << Profiler::FPS();

    m_camera.update(CameraSpeed);

    for (auto& object : m_physicsObjects)
    {
        object->update();
    }

    m_player.handleInput(m_world, m_physicsObjects);

    // worldのステップを進める
    m_world.step(Scene::DeltaTime());

    // 座標が一定以下ならオブジェクトを削除
    m_physicsObjects.remove_if([](const std::unique_ptr<PhysicsObject>& obj) { return obj->getPosition().y < -10.0; });

    // Tキーで爆発用のシーンへ移動
    if (KeyT.down())
    {
        changeScene(State::Explosion, 1.0s);
    }
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

void SceneGame::createStage()
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
}
