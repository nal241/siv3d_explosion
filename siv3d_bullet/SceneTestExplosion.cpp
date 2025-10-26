#include "SceneTestExplosion.h"
#include "SceneCommon.h"
#include "Renderers.h"
#include "ExplosiveEnemy.h"
#include "Bomb.h"

namespace
{
    // === シーン設定 ===
    constexpr double WallLength = 10.0;
    constexpr double WallThickness = 1.0;
    constexpr float WallRestitution = 1.0f;
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
}

void SceneTestExplosion::updateSceneSpecific()
{
    // マウスカーソルから静的オブジェクトへのレイキャスト
    const Ray ray = m_camera.screenToRay(Cursor::Pos());
    m_raycastResult = m_world.raycast(ray, MASK_STATIC_ONLY);

    // このシーン固有の表示
    // s3d::Print << U"Particles: {} "_fmt(m_particleSystem.m_particles.size());

    // Tキーでゲームシーンへ戻る
    if (KeyT.down())
    {
        changeScene(State::Game, 1.0s);
    }

    // Bキーで爆弾を投げる
    if (KeyB.down() && (m_throwCooldown.sF() >= 5.0 || !m_throwCooldown.isStarted()))
    {
        // マウスカーソル位置にレイがヒットしていたら
        if (m_raycastResult.hasHit)
        {
            const Vec3 startPos = m_camera.getEyePosition();
            const Vec3 targetPos = m_raycastResult.hitPoint;
            constexpr double launchAngle = -10.0;      // 角度を少し下げる
            const Vec3 gravity = m_world.getGravity(); // 物理ワールドの重力を取得

            // 投擲に必要な初速を計算
            if (auto launchVelocity = PhysicsWorld::CalculateLaunchVelocity(startPos, targetPos, launchAngle, gravity))
            {
                const float mass = 2.0f;
                const float radius = 0.4f;

                // 爆弾のパラメータを設定（発射位置はカメラの位置）
                Bomb::BombParams params{
                    .position = startPos,
                    .radius = radius,
                    .mass = mass,
                    .duration = 3.0, // 3秒後に爆発
                    .color = ColorF{1.0, 0.5, 0.2},
                    .restitution = 0.4f,
                    .friction = 0.8f,
                    .explosionRadius = 5.0, // 爆発半径5
                };

                // Bombファクトリを使ってオブジェクトを生成
                if (auto newBomb = Bomb::Create(m_world, params))
                {
                    // 計算された初速からインパルスを適用
                    const Vec3 impulse = *launchVelocity * mass;
                    newBomb->getPhysicsBody()->applyImpulse(impulse);

                    // シーンにオブジェクトを追加
                    addGameObject(std::move(newBomb));

                    // クールダウンを開始
                    m_throwCooldown.restart();
                }
            }
            else
            {
                // 到達不可能な位置への投擲を試みた場合
                Print << U"目標地点に到達できません";
            }
        }
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

        // 狙っている場所を可視化
        if (m_raycastResult.hasHit)
        {
            // ヒットした座標に小さな球を描画
            Sphere{m_raycastResult.hitPoint, 0.1}.draw(Palette::Red);

            // 地面にターゲットマーカーを描画
            Cylinder{m_raycastResult.hitPoint, 0.5, 0.05}.draw(ColorF{1.0, 0.5, 0.0, 0.5});
        }

        m_particleSystem.draw();
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
            m_instructionFont(U"B：爆弾を投げる").draw(30, 85, ColorF{1.0, 1.0, 1.0});
            m_instructionFont(U"T：ゲームシーンへ戻る").draw(30, 115, ColorF{1.0, 1.0, 1.0});
        }

        // クールダウンUIを描画
        {
            constexpr double cooldownTime = 5.0;
            const double progress = Min(m_throwCooldown.sF() / cooldownTime, 1.0);

            // 画面下部中央に配置
            const RectF bar{Arg::center(Scene::Center().x, Scene::Height() - 40), 400, 20};

            // 背景
            bar.draw(ColorF{0.0, 0.6});

            // 進捗
            bar.stretched(0, -(bar.w * (1.0 - progress)), 0, 0).draw(ColorF{0.9, 0.8, 0.3});

            // 枠線
            bar.drawFrame(1.5, ColorF{0.1});

            // テキスト（クールダウン完了時のみ表示）
            if (progress >= 1.0)
            {
                m_cooldownFont(U"BOMB READY").drawAt(bar.center(), ColorF{0.0});
            }
        }
    }
}
