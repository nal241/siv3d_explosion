#include "Explosion.h"

namespace
{
    // World settings（SceneGameと同じ）
    constexpr double WallLength = 10.0;
    constexpr double WallThickness = 1.0;
    constexpr float WallRestitution = 1.0f;

    // Camera settings（SceneGameと同じ）
    constexpr double CameraSpeed = 20.0;
    constexpr s3d::Vec3 CameraInitialPosition{5, 15, -20};
    constexpr s3d::Vec3 CameraInitialLookAt{5, 0, 10};
    constexpr double CameraFov = 30_deg;
}

Explosion::Explosion(const InitData& init)
    : IScene(init), m_renderTexture{Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes},
      m_player(&m_camera, m_model)
{
    Print << U"Explosion Scene Initialized";

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

void Explosion::update()
{
    // カメラを更新（マウスで視点を動かせる）
    m_camera.update(CameraSpeed);

    // 物理エンジンを更新
    m_world.step(Scene::DeltaTime());

    // 物理オブジェクトの位置を更新
    for (auto& object : m_physicsObjects)
    {
        object->update();
    }

    // Tキーでゲームのシーンへ移動
    if (KeyT.down())
    {
        changeScene(State::Game, 1.0s);
    }
}

void Explosion::draw() const
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

        // ★ UI を描画
        {
            // 半透明の背景パネル
            Rect{20, 20, 500, 120}.draw(ColorF{0.0, 0.0, 0.0, 0.7});

            // タイトル
            m_titleFont(U"これは爆発用のシーンです").draw(30, 30, ColorF{1.0, 0.7, 0.0}); // オレンジ色

            // 操作説明
            m_instructionFont(U"T：ゲームシーンへ戻る").draw(30, 85, ColorF{1.0, 1.0, 1.0}); // 白色
        }
    }
}
