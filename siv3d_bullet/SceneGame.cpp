#include "SceneGame.h"
#include "Renderers.h"
#include "Enemy.h"
#include "Bomb.h"

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

    // 吸引機能の設定
    constexpr double AttractionForce = 5.0;  // 吸引力の強さ（一定）
    constexpr double AttractionRadius = 2.0; // 吸引力の有効半径

    // === 爆発パーティクル設定 ===
    constexpr int32 ParticleCount = 50;
    constexpr double MinParticleSpeed = 3.0;
    constexpr double MaxParticleSpeed = 8.0;
    constexpr double MinParticleSize = 0.2;
    constexpr double MaxParticleSize = 0.5;
    constexpr double MinParticleLife = 0.8;
    constexpr double MaxParticleLife = 1.5;
    constexpr double MinParticleHue = 0.0;
    constexpr double MaxParticleHue = 60.0;
    constexpr double MinParticleSaturation = 0.7;
    constexpr double MaxParticleSaturation = 1.0;

    // === 爆発の物理パラメータ ===
    constexpr double ExplosionBasePower = 10.0;
    constexpr double ExplosionMinDistance = 0.01;
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
    updateCamera();
    updateInput();
    updateGameObjects();
    updatePhysics();
    updateParticleSystem();
    updateSpawn();
    removeObjects();
    updateSceneSpecific();
}

void SceneGame::updateCamera() { m_camera.update(CameraSpeed); }

void SceneGame::updateInput()
{
    ClearPrint();
    Print << U"Object num:{}"_fmt(m_gameObjects.size());
    Print << U"Tキーでシーン移動";
    Print << Profiler::FPS();

    // マウスカーソルから静的オブジェクトへのレイキャスト
    const Ray ray = m_camera.screenToRay(Cursor::Pos());
    m_raycastResult = m_world.raycast(ray, MASK_STATIC_ONLY);

    // 吸引処理
    if (MouseL.pressed() && m_raycastResult.hasHit)
    {
        const Vec3& hitPoint = m_raycastResult.hitPoint;

        for (const auto& object : m_gameObjects)
        {
            if (auto body = object->getPhysicsBody(); body && body->getGroup() == GROUP_ATTRACTABLE)
            {
                const Vec3 objPos = object->getPosition();
                const Vec3 direction = (hitPoint - objPos);
                const double distanceSq = direction.lengthSq();

                // 有効範囲内かチェック
                if (distanceSq < (AttractionRadius * AttractionRadius))
                {
                    const Vec3 force = direction.normalized() * AttractionForce;
                    body->applyForce(force);
                }
            }
        }
    }

    // プレイヤー入力
    m_player.handleInput(m_world, m_gameObjects);

    // シーン遷移
    if (KeyT.down())
    {
        changeScene(State::Explosion, 1.0s);
    }

    // Rキーでリザルトへ
    if (KeyR.down())
    {
        changeScene(State::Result);
    }
}

void SceneGame::updatePhysics() { m_world.step(static_cast<float>(Scene::DeltaTime())); }

void SceneGame::updateGameObjects()
{
    //全オブジェクトの状態更新
    for (const auto& object : m_gameObjects)
    {
        object->update();
    }

    //全イベントの処理
    for (const auto& object : m_gameObjects)
    {
        for (auto& event : object->consumeEvents())
        {
            std::visit(
                [this](auto&& e)
                {
                    using T = std::decay_t<decltype(e)>;
                    if constexpr (std::is_same_v<T, ExplosionRequest>)
                    {
                        handleExplosion(e);
                    }
                    // 将来: 他のイベント型を追加
                },
                event);
        }
    }
}

void SceneGame::updateParticleSystem() { m_particleSystem.update(Scene::DeltaTime()); }

void SceneGame::updateSpawn()
{
    if (m_enemySpawnTimer.sF() >= m_spawnInterval)
    {
        spawnEnemy();
        m_enemySpawnTimer.restart();
    }
}

void SceneGame::removeObjects()
{
    m_gameObjects.remove_if(
        [this](const std::shared_ptr<GameObject>& obj)
        {
            // 範囲外チェック
            bool shouldRemove = obj->getPosition().y < -10.0 || obj->shouldBeRemoved();

            if (shouldRemove)
            {
                // オブジェクト削除時にスコア加算
                getData().score += 10;
            }

            return shouldRemove;
        });
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

        m_particleSystem.draw();

        // --- デバッグ描画 ---

        // 吸引範囲の可視化
        if (MouseL.pressed() && m_raycastResult.hasHit)
        {
            const ScopedRenderStates3D blend{BlendState::OpaqueAlphaToCoverage};
            Sphere{m_raycastResult.hitPoint, AttractionRadius}.draw(ColorF{1.0, 0.5, 0.0, 0.5});
        }

        // レイキャストの結果を視覚化
        if (m_raycastResult.hasHit)
        {
            // ヒットしたオブジェクトをワイヤーフレームで描画
            if (auto hitObject = m_raycastResult.hitObject.lock())
            {
                hitObject->drawWireframe();
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

void SceneGame::spawnEnemy()
{
    // ステージ内のランダムな位置にスポーン
    const double x = Random(1.0, WallLength - 1.0);
    const double z = Random(1.0, WallLength - 1.0);
    const double y = 2.0;

    addGameObject(Enemy::Create(m_world, Enemy::EnemyParams{.position = Vec3{x, y, z},
                                                            .radius = 0.5f,
                                                            .mass = 2.0f,
                                                            .maxHealth = 100,
                                                            .color = HSV{0, 0.7, 0.9},
                                                            .group = GROUP_ATTRACTABLE,
                                                            .mask = MASK_ALL,
                                                            .explosionRadius = 3.0}));
}

void SceneGame::handleExplosion(const ExplosionRequest& request)
{
    // 爆発を実行（パーティクル + 物理的な力）
    createExplosionParticles(request.position, request.radius);
    applyExplosionForce(request);

    // サウンド再生
    m_explosionSound.playOneShot();
}

void SceneGame::createExplosionParticles(const s3d::Vec3& center, double radius)
{
    s3d::Print << U"   Creating {} particles"_fmt(ParticleCount);

    for (int32 i = 0; i < ParticleCount; ++i)
    {
        const double theta = s3d::Random(0.0, s3d::Math::TwoPi);
        const double phi = s3d::Random(0.0, s3d::Math::Pi);
        const double speed = s3d::Random(MinParticleSpeed, MaxParticleSpeed);

        s3d::Vec3 direction{s3d::Math::Sin(phi) * s3d::Math::Cos(theta), s3d::Math::Sin(phi) * s3d::Math::Sin(theta),
                            s3d::Math::Cos(phi)};

        Particle3D particle{.position = center,
                            .velocity = direction * speed,
                            .color = s3d::HSV{s3d::Random(MinParticleHue, MaxParticleHue),
                                              s3d::Random(MinParticleSaturation, MaxParticleSaturation), 1.0},
                            .size = s3d::Random(MinParticleSize, MaxParticleSize),
                            .life = s3d::Random(MinParticleLife, MaxParticleLife),
                            .active = true};
        m_particleSystem.add(particle);
    }
}

void SceneGame::applyExplosionForce(const ExplosionRequest& request)
{
    const s3d::Vec3& center = request.position;
    const double radius = request.radius;
    auto explosionSource = request.source.lock();

    s3d::Print << U"   Applying force to objects...";

    // 範囲内のオブジェクトを取得して力を加える
    auto nearbyResult = m_world.overlapSphere(center, radius, MASK_ALL);
    int32 hitCount = 0;

    for (auto weakObj : nearbyResult.hitObjects)
    {
        auto object = weakObj.lock();
        if (!object)
            continue;

        // 自分自身は除外
        if (explosionSource && object == explosionSource)
            continue;

        auto body = object->getPhysicsBody();
        if (!body || body->isStatic())
            continue;

        s3d::Vec3 objectPos = object->getPosition();
        s3d::Vec3 direction = objectPos - center;
        double distanceSq = direction.lengthSq();

        // 最小距離チェック
        const double minDistSq = ExplosionMinDistance * ExplosionMinDistance;
        if (distanceSq <= minDistSq)
            continue;

        // ここで一度だけ平方根を計算
        double distance = s3d::Math::Sqrt(distanceSq);
        s3d::Vec3 normalizedDirection = direction / distance; // 手動で正規化

        double falloff = 1.0 - (distance / radius);
        double explosionForce = ExplosionBasePower * falloff;
        s3d::Vec3 force = normalizedDirection * explosionForce;

        body->applyImpulse(force);
        hitCount++;

        // エネミーにダメージを与える
        if (auto enemy = std::dynamic_pointer_cast<Enemy>(object))
        {
            int damage = static_cast<int>(falloff * 100);
            enemy->takeDamage(damage);
            s3d::Print << U"  → Hit Enemy: distance {:.2f}, damage {}"_fmt(distance, damage);
        }
        else
        {
            s3d::Print << U"  → Hit: distance {:.2f}, force {:.2f}"_fmt(distance, explosionForce);
        }
    }
    s3d::Print << U"   Hit {} objects"_fmt(hitCount);
}
