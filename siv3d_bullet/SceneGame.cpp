#include "SceneGame.h"
#include "Renderers.h"

namespace
{
    // World settings
    constexpr double WallLength = 10.0;
    constexpr double WallThickness = 1.0;

    // Camera settings
    constexpr double CameraSpeed = 20.0;
    constexpr s3d::Vec3 CameraInitialPosition{5, 15, -20};
    constexpr s3d::Vec3 CameraInitialLookAt{5, 0, 10};
    constexpr double CameraFov = 30_deg;

    // ステージオブジェクトのプリセット
    constexpr float StaticBoxRestitution = 1.0f;
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

    // マウスカーソルからレイを発射
    const Ray ray = m_camera.screenToRay(Cursor::Pos());
    m_raycastResult = m_world.raycast(ray);

    // プレイヤー入力
    m_player.handleInput(m_world, m_gameObjects);

    // GameObjects更新
    for (const auto& object : m_gameObjects)
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
    m_gameObjects.remove_if([](const std::shared_ptr<GameObject>& obj) { return obj->getPosition().y < -10.0; });
}

void SceneGame::draw() const
{
    Graphics3D::SetCameraTransform(m_camera);

    // [3D rendering]
    {
        const ScopedRenderTarget3D target{m_renderTexture.clear(m_backgroundColor)};

        // for debug
        // Plane{64}.draw(uvChecker);

        for (const auto& object : m_gameObjects)
        {
            object->draw();
        }

        // レイキャストの結果を視覚化
        if (m_raycastResult.hasHit)
        {
            // ヒットしたオブジェクトをワイヤーフレームで描画
            if (m_raycastResult.hitObject)
            {
                m_raycastResult.hitObject->drawWireframe();
            }

            // ヒットした座標に小さな球を描画
            Sphere{m_raycastResult.hitPoint, 0.1}.draw(Palette::Red);

            // ヒットした座標の法線を描画
            const Vec3 normalEnd = m_raycastResult.hitPoint + m_raycastResult.hitNormal;
            Line3D{m_raycastResult.hitPoint, normalEnd}.draw(Palette::Yellow);
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

void SceneGame::addGameObject(std::shared_ptr<GameObject> obj) { m_gameObjects.push_back(std::move(obj)); }

void SceneGame::createStage()
{
    // Floor
    addGameObject(GameObject::CreateBox(
        m_world, GameObject::BoxParams{.size = s3d::Vec3(WallLength, WallThickness, WallLength),
                                       .position = s3d::Vec3(WallLength / 2, -WallThickness / 2, WallLength / 2),
                                       .mass = 0.0f, // 静的オブジェクト
                                       .color = s3d::Linear::Palette::Silver,
                                       .restitution = StaticBoxRestitution}));

    // Left Wall
    addGameObject(GameObject::CreateBox(
        m_world, GameObject::BoxParams{.size = s3d::Vec3(WallThickness, WallLength, WallLength),
                                       .position = s3d::Vec3(-WallThickness / 2, WallLength / 2, WallLength / 2),
                                       .mass = 0.0f,
                                       .color = s3d::Linear::Palette::Powderblue,
                                       .restitution = StaticBoxRestitution}));

    // Right Wall
    addGameObject(GameObject::CreateBox(
        m_world,
        GameObject::BoxParams{.size = s3d::Vec3(WallThickness, WallLength, WallLength),
                              .position = s3d::Vec3(WallLength + WallThickness / 2, WallLength / 2, WallLength / 2),
                              .mass = 0.0f,
                              .color = s3d::Linear::Palette::Powderblue,
                              .restitution = StaticBoxRestitution}));

    // Back Wall
    addGameObject(GameObject::CreateBox(
        m_world,
        GameObject::BoxParams{.size = s3d::Vec3(WallLength, WallLength, WallThickness),
                              .position = s3d::Vec3(WallLength / 2, WallLength / 2, WallLength + WallThickness / 2),
                              .mass = 0.0f,
                              .color = s3d::Linear::Palette::Powderblue,
                              .restitution = StaticBoxRestitution}));
}
