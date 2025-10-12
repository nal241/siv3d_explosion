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

	// Bomb settings
    constexpr double BombRadius = 0.5;           // 爆弾の半径
    constexpr s3d::Vec3 BombPosition{5, 0.5, 5}; // 床の中央、少し浮かせる
    constexpr float BombMass = 0.0f;            // 爆弾の質量
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

	// ★ 爆弾を作成（床の中央に配置）
    auto bomb = m_world.createSphere(SphereDesc{BombRadius, BombPosition, BombMass});
    bomb->setRestitution(0.0f);
    bomb->setColor(ColorF{0.1, 0.1, 0.1}); // 黒色

    // 爆弾のポインタを保存
    m_bomb = bomb.get();

	// ★ テスト用のキューブを爆弾の周りに配置
    for (int i = 0; i < 8; i++)
    {
        double angle = i * (Math::TwoPi / 8);
        double distance = 3.0;

        Vec3 position{5 + Math::Cos(angle) * distance,
                      1.0, // 床から1メートル上
                      5 + Math::Sin(angle) * distance};

        auto testBox = m_world.createBox(BoxDesc{Vec3{0.5, 0.5, 0.5}, position, 2.0f});
        testBox->setRestitution(0.5f);
        testBox->setColor(HSV{i * 45, 0.7, 0.9});

        m_physicsObjects.push_back(std::move(testBox));
    }

    m_physicsObjects.push_back(std::move(floor));
    m_physicsObjects.push_back(std::move(wall_l));
    m_physicsObjects.push_back(std::move(wall_r));
    m_physicsObjects.push_back(std::move(wall_b));
    m_physicsObjects.push_back(std::move(bomb)); // 爆弾も追加

    m_camera = DebugCamera3D{m_renderTexture.size(), CameraFov, CameraInitialPosition, CameraInitialLookAt};

    for (auto& object : m_physicsObjects)
    {
        object->update();
    }
}

void Explosion::update()
{
    ClearPrint();
    Print << U"Object num:{}"_fmt(m_physicsObjects.size());
    Print << Profiler::FPS();
    // カメラを更新（マウスで視点を動かせる）
    m_camera.update(CameraSpeed);

    // 物理オブジェクトの位置を更新
    for (auto& object : m_physicsObjects)
    {
        object->update();
    }

	// TODO: player
    m_player.handleInput(m_world, m_physicsObjects);

    // worldのステップを進める
    m_world.step(Scene::DeltaTime());

    // 座標が一定以下ならオブジェクトを削除
    m_physicsObjects.remove_if([](const std::unique_ptr<PhysicsObject>& obj) { return obj->getPosition().y < -10.0; });


	// ★ Pキーで爆発
    if (KeyP.down() && m_bomb != nullptr)
    {
        explode(m_bomb, 5.0); // 半径5メートルの爆発
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
            m_instructionFont(U"P：爆発させる").draw(30, 115, ColorF{1.0, 1.0, 1.0});        // 白色
        }
    }
}

// ★ 爆発関数の実装
void Explosion::explode(PhysicsObject* bomb, double radius)
{
    if (!bomb)
        return;

	// ★ 爆発音を再生
    m_explosionSound.playOneShot();

    // 爆弾の中心位置を取得
    Vec3 bombCenter = bomb->getPosition();

    Print << U"💥 Explosion at {}"_fmt(bombCenter);
    Print << U"Radius: {}"_fmt(radius);

    // すべてのオブジェクトをチェック
    for (auto& object : m_physicsObjects)
    {
        // 爆弾自身はスキップ
        if (object.get() == bomb)
            continue;

        // 静止オブジェクト（質量0）はスキップ
        if (object->getMass() == 0.0f)
            continue;

        // オブジェクトの位置を取得
        Vec3 objectPos = object->getPosition();

        // 爆弾からオブジェクトへのベクトル
        Vec3 direction = objectPos - bombCenter;

        // 距離を計算
        double distance = direction.length();

        // 爆発半径内にあるかチェック
        if (distance < radius)
        {
            // 方向ベクトルを正規化
            Vec3 normalizedDirection = direction.normalized();

            // 距離に応じて力を減衰（近いほど強い）
            double falloff = 1.0 - (distance / radius); // 0.0 ～ 1.0

            // 爆発力を計算
            double explosionForce = 100.0 * falloff; // 基本力 × 減衰

            // 力のベクトル
            Vec3 force = normalizedDirection * explosionForce;

            // 力を加える
            object->applyImpulse(force);

            Print << U"  → Hit object at distance {:.2f}, force: {:.2f}"_fmt(distance, explosionForce);
        }
    }
}
