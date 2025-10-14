#include "Explosion.h"

namespace
{
    constexpr double WallLength = 10.0;
    constexpr double WallThickness = 1.0;
    constexpr float WallRestitution = 1.0f;
    constexpr double CameraSpeed = 20.0;
    constexpr s3d::Vec3 CameraInitialPosition{5, 15, -20};
    constexpr s3d::Vec3 CameraInitialLookAt{5, 0, 10};
    constexpr double CameraFov = 30_deg;
    constexpr double BombRadius = 0.5;
    constexpr s3d::Vec3 BombPosition{5, 0.5, 5};
    constexpr float BombMass = 0.0f;
} // namespace

Explosion::Explosion(const InitData& init)
    : IScene(init), m_renderTexture{Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes},
      m_player(&m_camera, m_model)
{
    Print << U"Explosion Scene Initialized";

    // 床、壁、爆弾、テストキューブの作成（既存のコードと同じ）
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

    auto bomb = m_world.createSphere(SphereDesc{BombRadius, BombPosition, BombMass});
    bomb->setRestitution(0.0f);
    bomb->setColor(ColorF{0.1, 0.1, 0.1});
    m_bomb = bomb.get();

    for (int i = 0; i < 8; i++)
    {
        double angle = i * (Math::TwoPi / 8);
        double distance = 3.0;

        Vec3 position{5 + Math::Cos(angle) * distance, 1.0, 5 + Math::Sin(angle) * distance};

        auto testBox = m_world.createBox(BoxDesc{Vec3{0.5, 0.5, 0.5}, position, 2.0f});
        testBox->setRestitution(0.5f);
        testBox->setColor(HSV{i * 45, 0.7, 0.9});

        auto id = testBox->getID();
        m_physicsObjects.emplace(id, std::move(testBox));
    }

    auto floorID = floor->getID();
    m_physicsObjects.emplace(floorID, std::move(floor));

    auto wallLID = wall_l->getID();
    m_physicsObjects.emplace(wallLID, std::move(wall_l));

    auto wallRID = wall_r->getID();
    m_physicsObjects.emplace(wallRID, std::move(wall_r));

    auto wallBID = wall_b->getID();
    m_physicsObjects.emplace(wallBID, std::move(wall_b));

    auto bombID = bomb->getID();
    m_physicsObjects.emplace(bombID, std::move(bomb));

    m_camera = DebugCamera3D{m_renderTexture.size(), CameraFov, CameraInitialPosition, CameraInitialLookAt};

    for (auto& [id, object] : m_physicsObjects)
    {
        object->update();
    }
}

void Explosion::update()
{
    ClearPrint();
    Print << U"Object num: {}"_fmt(m_physicsObjects.size());
    Print << U"Particles: {}"_fmt(m_particles.size());
    Print << Profiler::FPS();

    m_camera.update(CameraSpeed);

    // ★ パーティクルを更新
    const double deltaTime = Scene::DeltaTime();

    for (auto& particle : m_particles)
    {
        if (!particle.active)
            continue;

        // 速度を更新（重力を適用）
        particle.velocity += Gravity * deltaTime;

        // 位置を更新
        particle.position += particle.velocity * deltaTime;

        // 寿命を減らす
        particle.life -= deltaTime;

        // 寿命が尽きたら非アクティブに
        if (particle.life <= 0.0)
        {
            particle.active = false;
        }
    }

    // 非アクティブなパーティクルを削除
    m_particles.remove_if([](const Particle3D& p) { return !p.active; });

    // 物理オブジェクトの位置を更新
    for (auto& [id, object] : m_physicsObjects)
    {
        object->update();
    }

    // プレイヤー入力
    m_player.handleInput(m_world, m_physicsObjects);

    // 物理エンジンを更新
    m_world.step(deltaTime);

    // 削除対象のIDを集める
    Array<PhysicsObject::IDType> toRemove;
    for (const auto& [id, object] : m_physicsObjects)
    {
        if (object->getPosition().y < -10.0)
        {
            toRemove.push_back(id);
        }
    }

    // 削除実行
    for (const auto& id : toRemove)
    {
        m_physicsObjects.erase(id);
    }

    // Pキーで爆発
    if (KeyP.down() && m_bomb != nullptr)
    {
        explode(m_bomb, 5.0);
    }

    // Tキーでゲームシーンへ戻る
    if (KeyT.down())
    {
        changeScene(State::Game, 1.0s);
    }
}

void Explosion::draw() const
{
    Graphics3D::SetCameraTransform(m_camera);

    // [3D rendering]
    {
        const ScopedRenderTarget3D target{m_renderTexture.clear(m_backgroundColor)};

        // 3Dオブジェクトを描画
        for (const auto& [id, object] : m_physicsObjects)
        {
            object->draw();
        }

        // ★ 3D空間にパーティクルを描画（加算ブレンドで光らせる）
        {
            const ScopedRenderStates3D blend{BlendState::Additive};

            for (const auto& particle : m_particles)
            {
                if (!particle.active)
                    continue;

                // 寿命に応じて透明度を変化
                const double alpha = particle.life;
                const ColorF color = particle.color.withAlpha(alpha).removeSRGBCurve();

                // 球として描画
                Sphere{particle.position, particle.size}.draw(color);
            }
        }
    }

    // [2D rendering]
    {
        Graphics3D::Flush();
        m_renderTexture.resolve();
        Shader::LinearToScreen(m_renderTexture);

        // UI を描画
        {
            Rect{20, 20, 500, 150}.draw(ColorF{0.0, 0.0, 0.0, 0.7});

            m_titleFont(U"これは爆発用のシーンです").draw(30, 30, ColorF{1.0, 0.7, 0.0});

            m_instructionFont(U"P：爆発させる").draw(30, 85, ColorF{1.0, 1.0, 1.0});

            m_instructionFont(U"T：ゲームシーンへ戻る").draw(30, 115, ColorF{1.0, 1.0, 1.0});
        }
    }
}

void Explosion::explode(PhysicsObject* bomb, double radius)
{
    if (!bomb)
        return;

    // 爆発音を再生
    m_explosionSound.playOneShot();

    // 爆弾の中心位置を取得
    Vec3 bombCenter = bomb->getPosition();

    Print << U"💥 Explosion at {}"_fmt(bombCenter);
    Print << U"Radius: {}"_fmt(radius);

    // ★ 3D空間にパーティクルを生成（50個）
    for (int32 i = 0; i < 50; ++i)
    {
        // 球状にランダムな方向
        const double theta = Random(0.0, Math::TwoPi);
        const double phi = Random(0.0, Math::Pi);
        const double speed = Random(3.0, 8.0);

        Vec3 direction{Math::Sin(phi) * Math::Cos(theta), Math::Sin(phi) * Math::Sin(theta), Math::Cos(phi)};

        Particle3D particle{.position = bombCenter,
                            .velocity = direction * speed,
                            .color = HSV{Random(0.0, 60.0), Random(0.7, 1.0), 1.0},
                            .size = Random(0.2, 0.5),
                            .life = Random(0.8, 1.5),
                            .active = true};

        m_particles << particle;
    }

    Print << U"   Created {} particles"_fmt(50);

    // 物理演算：オブジェクトに力を加える
    for (auto& [id, object] : m_physicsObjects)
    {
        if (object.get() == bomb)
            continue;

        if (object->getMass() == 0.0f)
            continue;

        Vec3 objectPos = object->getPosition();
        Vec3 direction = objectPos - bombCenter;
        double distance = direction.length();

        if (distance < radius && distance > 0.01)
        {
            Vec3 normalizedDirection = direction.normalized();
            double falloff = 1.0 - (distance / radius);
            double explosionForce = 500.0 * falloff;
            Vec3 force = normalizedDirection * explosionForce;

            object->applyImpulse(force);

            Print << U"  → Hit: distance {:.2f}, force {:.2f}"_fmt(distance, explosionForce);
        }
    }
}
