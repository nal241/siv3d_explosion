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
    Print << U"Object num:{}"_fmt(m_gameObjects.size());
    Print << U"Tキーで爆発用のシーンへ移動";
    Print << Profiler::FPS();

    m_camera.update(CameraSpeed);

    for (auto const& [id, object] : m_gameObjects)
    {
        object->update();
    }

    m_player.handleInput(m_world, m_gameObjects);

    // worldのステップを進める
    m_world.step(Scene::DeltaTime());

    // 座標が一定以下ならオブジェクトを削除
    {
        Array<GameObject::IDType> objectsToRemove;
        for (const auto& [id, object] : m_gameObjects)
        {
            if (object->getPosition().y < -10.0)
            {
                objectsToRemove.push_back(id);
            }
        }

        for (const auto& id : objectsToRemove)
        {
            m_gameObjects.erase(id);
        }
    }

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
        for (auto const& [id, object] : m_gameObjects)
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

    auto floorObj = std::make_unique<GameObject>();
    floorObj->setPhysicsBody(std::move(floor));
    floorObj->setColor(s3d::Linear::Palette::Silver);

    auto wall_l = m_world.createBox(BoxDesc{s3d::Vec3(WallThickness, WallLength, WallLength),
                                            s3d::Vec3(-WallThickness / 2, WallLength / 2, WallLength / 2), 0.0f});
    wall_l->setRestitution(WallRestitution);

    auto wallLObj = std::make_unique<GameObject>();
    wallLObj->setPhysicsBody(std::move(wall_l));
    wallLObj->setColor(s3d::Linear::Palette::Powderblue);

    auto wall_r =
        m_world.createBox(BoxDesc{s3d::Vec3(WallThickness, WallLength, WallLength),
                                  s3d::Vec3(WallLength + WallThickness / 2, WallLength / 2, WallLength / 2), 0.0f});
    wall_r->setRestitution(WallRestitution);

    auto wallRObj = std::make_unique<GameObject>();
    wallRObj->setPhysicsBody(std::move(wall_r));
    wallRObj->setColor(s3d::Linear::Palette::Powderblue);

    auto wall_b =
        m_world.createBox(BoxDesc{s3d::Vec3(WallLength, WallLength, WallThickness),
                                  s3d::Vec3(WallLength / 2, WallLength / 2, WallLength + WallThickness / 2), 0.0f});
    wall_b->setRestitution(WallRestitution);

    auto wallBObj = std::make_unique<GameObject>();
    wallBObj->setPhysicsBody(std::move(wall_b));
    wallBObj->setColor(s3d::Linear::Palette::Powderblue);

    m_gameObjects.emplace(floorObj->getID(), std::move(floorObj));
    m_gameObjects.emplace(wallLObj->getID(), std::move(wallLObj));
    m_gameObjects.emplace(wallRObj->getID(), std::move(wallRObj));
    m_gameObjects.emplace(wallBObj->getID(), std::move(wallBObj));
}
