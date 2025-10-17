#include "SceneGame.h"
#include "Renderers.h"

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

    // カメラ更新
    m_camera.update(CameraSpeed);

    // プレイヤー入力
    m_player.handleInput(m_world, m_gameObjects);

    // GameObjects更新
    for (auto const& [_, object] : m_gameObjects)
    {
        object->update();
    }

    // 物理演算
    m_world.step(Scene::DeltaTime());

    // 範囲外オブジェクト削除
    removeOutOfBoundsObjects();

    // シーン遷移
    if (KeyT.down())
    {
        changeScene(State::Explosion, 1.0s);
    }
}

void SceneGame::removeOutOfBoundsObjects()
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

void SceneGame::draw() const
{
    Graphics3D::SetCameraTransform(m_camera);

    // [3D rendering]
    {
        const ScopedRenderTarget3D target{m_renderTexture.clear(m_backgroundColor)};

        // for debug
        // Plane{64}.draw(uvChecker);

        for (auto const& [_, object] : m_gameObjects)
        {
            object->draw();
        }
    }

    // [2D rendering]
    {
        Graphics3D::Flush();
        m_renderTexture.resolve();
        Shader::LinearToScreen(m_renderTexture);
    }
}

std::unique_ptr<GameObject> SceneGame::createStaticWall(const Vec3& size, const Vec3& position, const ColorF& color)
{
    auto body = m_world.createBox(BoxDesc{size, position, 0.0f});
    body->setRestitution(WallRestitution);

    auto obj = std::make_unique<GameObject>();
    obj->setRenderer(std::make_unique<PhysicsShapeRenderer>(*body, color));
    obj->setPhysicsBody(std::move(body));

    return obj;
}

void SceneGame::addGameObject(std::unique_ptr<GameObject> obj) { m_gameObjects.emplace(obj->getID(), std::move(obj)); }

void SceneGame::createStage()
{
    // Floor
    addGameObject(createStaticWall(s3d::Vec3(WallLength, WallThickness, WallLength),
                                   s3d::Vec3(WallLength / 2, -WallThickness / 2, WallLength / 2),
                                   s3d::Linear::Palette::Silver));

    // Left Wall
    addGameObject(createStaticWall(s3d::Vec3(WallThickness, WallLength, WallLength),
                                   s3d::Vec3(-WallThickness / 2, WallLength / 2, WallLength / 2),
                                   s3d::Linear::Palette::Powderblue));

    // Right Wall
    addGameObject(createStaticWall(s3d::Vec3(WallThickness, WallLength, WallLength),
                                   s3d::Vec3(WallLength + WallThickness / 2, WallLength / 2, WallLength / 2),
                                   s3d::Linear::Palette::Powderblue));

    // Back Wall
    addGameObject(createStaticWall(s3d::Vec3(WallLength, WallLength, WallThickness),
                                   s3d::Vec3(WallLength / 2, WallLength / 2, WallLength + WallThickness / 2),
                                   s3d::Linear::Palette::Powderblue));
}
