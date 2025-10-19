#include "SceneTestExplosion.h"
#include "Renderers.h"
#include "Enemy.h"

namespace
{
    // === シーン設定 ===
    constexpr double WallLength = 10.0;
    constexpr double WallThickness = 1.0;
    constexpr float WallRestitution = 1.0f;
    constexpr double CameraSpeed = 20.0;
    constexpr s3d::Vec3 CameraInitialPosition{5, 15, -20};
    constexpr s3d::Vec3 CameraInitialLookAt{5, 0, 10};
    constexpr double CameraFov = 30_deg;

    // === 爆弾設定 ===
    constexpr double BombRadius = 0.5;
    constexpr s3d::Vec3 BombPosition{5, 0.5, 5};
    constexpr float BombMass = 0.0f;

    // === パーティクル設定 ===
    constexpr int32 ParticleCount = 50;           // 1回の爆発で生成するパーティクル数
    constexpr double MinParticleSpeed = 3.0;      // パーティクルの最小初速（m/s）
    constexpr double MaxParticleSpeed = 8.0;      // パーティクルの最大初速（m/s）
    constexpr double MinParticleSize = 0.2;       // パーティクルの最小サイズ（m）
    constexpr double MaxParticleSize = 0.5;       // パーティクルの最大サイズ（m）
    constexpr double MinParticleLife = 0.8;       // パーティクルの最小寿命（秒）
    constexpr double MaxParticleLife = 1.5;       // パーティクルの最大寿命（秒）
    constexpr double MinParticleHue = 0.0;        // パーティクルの色相の最小値
    constexpr double MaxParticleHue = 60.0;       // パーティクルの色相の最大値（オレンジ～赤）
    constexpr double MinParticleSaturation = 0.7; // パーティクルの彩度の最小値
    constexpr double MaxParticleSaturation = 1.0; // パーティクルの彩度の最大値

    // === 爆発の物理パラメータ ===
    constexpr double ExplosionBasePower = 10.0;   // 爆発の基本威力
    constexpr double ExplosionMinDistance = 0.01; // これ以下の距離では力を加えない（ゼロ除算防止）
} // namespace

SceneTestExplosion::SceneTestExplosion(const InitData& init) : SceneGame(init)
{
    s3d::Print << U"Explosion Scene Initialized";

    // --- オブジェクト生成 ---

    // 爆弾
    {
        auto bomb = GameObject::CreateSphere(m_world, GameObject::SphereParams{.radius = BombRadius,
                                                                               .position = BombPosition,
                                                                               .mass = BombMass,
                                                                               .color = ColorF{0.1, 0.1, 0.1},
                                                                               .restitution = 0.0f});
        m_bombObject = bomb; // weak_ptrに保存
        addGameObject(std::move(bomb));
    }

    // テストキューブ
    for (int i = 0; i < 8; i++)
    {
        double angle = i * (Math::TwoPi / 8);
        double distance = 3.0;
        Vec3 position{5 + Math::Cos(angle) * distance, 1.0, 5 + Math::Sin(angle) * distance};

        addGameObject(GameObject::CreateBox(m_world, GameObject::BoxParams{.size = Vec3{0.5, 0.5, 0.5},
                                                                           .position = position,
                                                                           .mass = 2.0f,
                                                                           .color = HSV{i * 45, 0.7, 0.9},
                                                                           .restitution = 0.5f}));
    }

    m_camera = DebugCamera3D{m_renderTexture.size(), CameraFov, CameraInitialPosition, CameraInitialLookAt};
}

void SceneTestExplosion::updateSceneSpecific()
{
    // パーティクルを更新
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

    // このシーン固有の表示
    s3d::Print << U"Particles: {}"_fmt(m_particles.size());

    // Pキーで爆発
    if (KeyP.down())
    {
        if (auto bomb = m_bombObject.lock()) // 生存確認
        {
            explode(bomb, 5.0);
        }
    }

    // Tキーでゲームシーンへ戻る
    if (KeyT.down())
    {
        changeScene(State::Game, 1.0s);
    }
}

void SceneTestExplosion::draw() const
{
    Graphics3D::SetCameraTransform(m_camera);

    // [3D rendering]
    {
        const ScopedRenderTarget3D target{m_renderTexture.clear(m_backgroundColor)};

        // 3Dオブジェクトを描画
        for (const auto& object : m_gameObjects)
        {
            object->draw();
        }

        // 3D空間にパーティクルを描画（加算ブレンドで光らせる）
        {
            const ScopedRenderStates3D blend{BlendState::Additive};
            for (const auto& particle : m_particles)
            {
                if (!particle.active)
                    continue;

                // 寿命に応じて透明度を変化
                const double alpha = particle.life;
                // リニアレンダリング用なのでremoveSRGBCurve()でsRGBカーブを除去
                // 参考: https://zenn.dev/reputeless/books/siv3d-documentation/viewer/tutorial-3d
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

void SceneTestExplosion::explode(const std::shared_ptr<GameObject>& bomb, double radius)
{
    if (!bomb)
        return;

    // 爆発音を再生
    m_explosionSound.playOneShot();

    // 爆弾の中心位置を取得
    Vec3 bombCenter = bomb->getPosition();

    s3d::Print << U"💥 Explosion at {}"_fmt(bombCenter);
    s3d::Print << U"Radius: {}"_fmt(radius);

    // === パーティクル生成 ===
    for (int32 i = 0; i < ParticleCount; ++i)
    {
        // 球状にランダムな方向
        const double theta = Random(0.0, Math::TwoPi);
        const double phi = Random(0.0, Math::Pi);
        const double speed = Random(MinParticleSpeed, MaxParticleSpeed);

        Vec3 direction{Math::Sin(phi) * Math::Cos(theta), Math::Sin(phi) * Math::Sin(theta), Math::Cos(phi)};

        Particle3D particle{.position = bombCenter,
                            .velocity = direction * speed,
                            .color = HSV{Random(MinParticleHue, MaxParticleHue),
                                         Random(MinParticleSaturation, MaxParticleSaturation), 1.0},
                            .size = Random(MinParticleSize, MaxParticleSize),
                            .life = Random(MinParticleLife, MaxParticleLife),
                            .active = true};

        m_particles << particle;
    }

    s3d::Print << U"   Created {} particles"_fmt(ParticleCount);

    // === 物理演算：オブジェクトに力を加える ===
    int32 hitCount = 0;

    for (auto& object : m_gameObjects)
    {
        // 爆弾自身はスキップ
        if (object == bomb)
            continue;

        // GameObjectからPhysicsBodyを取得
        auto body = object->getPhysicsBody();
        if (!body || body->isStatic())
            continue;

        Vec3 objectPos = object->getPosition();
        Vec3 direction = objectPos - bombCenter;
        double distance = direction.length();

        // 範囲内かつ有効な距離の場合のみ力を加える
        if (distance < radius && distance > ExplosionMinDistance)
        {
            Vec3 normalizedDirection = direction.normalized();
            double falloff = 1.0 - (distance / radius);
            double explosionForce = ExplosionBasePower * falloff;
            Vec3 force = normalizedDirection * explosionForce;

            body->applyImpulse(force);
            hitCount++;

            // エネミーにダメージを与える
            if (auto enemy = std::dynamic_pointer_cast<Enemy>(object))
            {
                int damage = static_cast<int>(falloff * 100); // 最大100ダメージ
                enemy->takeDamage(damage);
                s3d::Print << U"  → Hit Enemy: distance {:.2f}, damage {}"_fmt(distance, damage);
            }
            else
            {
                s3d::Print << U"  → Hit: distance {:.2f}, force {:.2f}"_fmt(distance, explosionForce);
            }
        }
    }

    s3d::Print << U"   Hit {} objects"_fmt(hitCount);
}
