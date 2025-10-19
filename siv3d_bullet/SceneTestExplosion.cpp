#include "SceneTestExplosion.h"
#include "SceneCommon.h"
#include "Renderers.h"
#include "Enemy.h"
#include "ExplosionHelper.h"

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
            // 爆発音を再生
            m_explosionSound.playOneShot();

            s3d::Print << U"💥 Explosion at {} with radius 5.0"_fmt(bomb->getPosition());

            ExplosionHelper::CreateExplosion(m_particles, m_gameObjects, bomb->getPosition(), 5.0, bomb);
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


