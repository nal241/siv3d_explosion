#include "SceneGame.h"
#include "Renderers.h"
#include "EnemyExplosive.h"
#include "Bomb.h"
#include "EnemyNormal.h"
#include "Stage.h"

namespace
{
    // World settings
    constexpr double WallThickness = 1.0;

    // Camera settings

    constexpr double CameraSpeed = 20.0;
    constexpr s3d::Vec3 CameraInitialPosition{0, 10, -5};
    constexpr s3d::Vec3 CameraInitialLookAt{0, 0, 40};
    constexpr double CameraFov = 30_deg;

    // ステージオブジェクトのプリセット
    constexpr float StaticBoxRestitution = 1.0f;

    // 吸引機能の設定
    constexpr double AttractionForce = 10.0;     // 吸引力の強さ（一定）
    constexpr double AttractionRadius = 5.0;     // 吸引力の有効半径
    constexpr double GravityFieldDuration = 3.0; // 重力場の持続時間

    // === アイテムエフェクト色設定 ===
    const ColorF GravityEffectColor{0.3, 0.1, 0.5};
    const ColorF FreezeEffectColor{0.5, 0.8, 1.0};
    const ColorF WindEffectColor{0.3, 1.0, 0.3};
    const ColorF BombIndicatorColor{1.0, 0.4, 0.2};

    // === アイテムインジケータ設定 ===
    constexpr double IndicatorHeight = 0.05;
    constexpr Duration IndicatorPulseDuration = 1.0s;
    constexpr double IndicatorPulseMaxAlpha = 0.8;
    constexpr double IndicatorPulseMinAlpha = 0.4;

    // Freezeアイテムの設定
    constexpr double FreezeRadius = 5.0;
    constexpr double FreezeDuration = 8.0;

    // === 氷柱エフェクト設定 ===
    constexpr int32 MinSpikeCount = 8;
    constexpr int32 MaxSpikeCount = 12;
    constexpr double MinSpikeHeight = 1.5;
    constexpr double MaxSpikeHeight = 2.5;
    constexpr double MinSpikeRadius = 0.2;
    constexpr double MaxSpikeRadius = 0.4;
    constexpr double SpikeGrowDuration = 0.5;
    constexpr double SpikeDirectionRandomness = 0.5;
    constexpr double SpikeAlpha = 0.6;
    constexpr double FrostWaveHeight = 0.01;
    constexpr double FrostWaveOffsetY = 0.01;



    // 風の設定
    constexpr double WindBoxWidth = 8.0;
    constexpr double WindBoxHeight = 2.0;
    constexpr double WindBoxDepth = 30.0;
    constexpr double WindForce = 15.0;
    constexpr double WindDuration = 3.0;

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

    // === 氷結晶パーティクル設定 ===
    constexpr int32 FreezeParticleCount = 50;
    constexpr double FreezeParticleMinSpeedY = 1.0;
    constexpr double FreezeParticleMaxSpeedY = 5.0;
    constexpr double FreezeParticleHorizontalRadius = 5.0;
    const ColorF FreezeParticleColor{0.7, 0.9, 1.0};
    constexpr double FreezeParticleMinSize = 0.01;
    constexpr double FreezeParticleMaxSize = 0.05;
    constexpr double FreezeParticleMinLife = 1.0;
    constexpr double FreezeParticleMaxLife = 1.5;

    // === 爆発の物理パラメータ ===
    constexpr double ExplosionBasePower = 10.0;
    constexpr double ExplosionMinDistance = 0.01;

    // === 画面揺れ設定 ===
    constexpr double ShakeSpeed = 10.0;
    constexpr double ExplosionShakeDuration = 0.5;
    constexpr double ExplosionShakeMagnitude = 0.5;
} // namespace

SceneGame::SceneGame(const InitData& init)
    : IScene(init), m_renderTexture{Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes},
      m_player(&m_camera, m_model),
      m_enemyNormalModel{U"LicensedAsset/normalEnemy.obj"},
      m_enemyExplosiveModel{U"LicensedAsset/enemyExplosive.obj"},
      m_launchBombSound{U"LicensedAsset/launchBomb.mp3"},
      m_gravitySound{U"LicensedAsset/gravity.mp3"},
      m_freezeSound{U"LicensedAsset/freeze.mp3"},
      m_windSound{U"LicensedAsset/wind.mp3"},
      m_bgm{U"LicensedAsset/BGM_LessVolume.m4a", Loop::Yes},
      m_frostTexture{U"LicensedAsset/Snow.jpg", TextureDesc::MippedSRGB},
      m_darknessTexture{U"LicensedAsset/darkness.jpg", TextureDesc::MippedSRGB},
      m_windTexture{U"LicensedAsset/wind.jpg", TextureDesc::MippedSRGB}
{
    Model::RegisterDiffuseTextures(m_enemyNormalModel, TextureDesc::MippedSRGB);
    Model::RegisterDiffuseTextures(m_enemyExplosiveModel, TextureDesc::MippedSRGB);

    // stage作成
    createStage();

    // カメラ設定
    // m_camera = BasicCamera3D{m_renderTexture.size(), CameraFov};
    m_camera = DebugCamera3D{m_renderTexture.size(), CameraFov};
    m_cameraPosition = CameraInitialPosition;
    m_cameraLookAt = CameraInitialLookAt;
    m_camera.setView(m_cameraPosition, m_cameraLookAt);

    // 揺れノイズの初期化
    m_noiseSeeds = s3d::Vec3{s3d::Random(100.0, 999.0), s3d::Random(100.0, 999.0), s3d::Random(100.0, 999.0)};

    // デバッグ描画の設定
    m_world.setDebugDrawMode(btIDebugDraw::DBG_DrawWireframe);
    m_world.setDebugDrawEnabled(true); // デフォルトで有効化

    m_bgm.play();
}

void SceneGame::update()
{
    updateCamera();
    updateInput();
    updateUI();
    updateItems();
    updateGameObjects();
    updatePhysics();
    updateParticleSystem();
    updateSpawn();
    removeObjects();
}

void SceneGame::updateCamera()
{
    m_camera.update(20.0);
    const double elapsed = m_shakeTimer.sF();

    if (not m_shakeTimer.isStarted() || elapsed >= m_shakeDuration)
    {
        m_camera.setView(m_cameraPosition, m_cameraLookAt);
        return;
    }

    // 時間経過とともに揺れを減衰させる
    const double currentMagnitude = m_shakeMagnitude * (1.0 - (elapsed / m_shakeDuration));

    // Perlinノイズを使って滑らかな揺れを生成
    m_shakeNoiseTime += Scene::DeltaTime() * ShakeSpeed;

    const double x = m_shakeNoise.noise2D(m_shakeNoiseTime, m_noiseSeeds.x) * currentMagnitude;
    const double y = m_shakeNoise.noise2D(m_shakeNoiseTime, m_noiseSeeds.y) * currentMagnitude;
    const double z = m_shakeNoise.noise2D(m_shakeNoiseTime, m_noiseSeeds.z) * currentMagnitude;

    const Vec3 finalPosition = m_cameraPosition + Vec3{x, y, z};
    const Vec3 finalCameraLookAt = m_cameraLookAt + Vec3{x, y, z};
    m_camera.setView(finalPosition, finalCameraLookAt);
}

void SceneGame::updateInput()
{
    Logger << U"Object num:{}"_fmt(m_gameObjects.size());
    Logger << Profiler::FPS();

    // プレイヤー入力
    m_player.handleInput(m_world, m_gameObjects);

    // Dキーでデバッグ描画切り替え
    if (KeyD.down())
    {
        m_debugDrawEnabled = !m_debugDrawEnabled;
        m_world.setDebugDrawEnabled(m_debugDrawEnabled);
    }

    // Tキーでタイトルへ
    if (KeyT.down())
    {
        changeScene(State::Title, 1.0s);
    }

    // Rキーでリザルトへ
    if (KeyR.down())
    {
        changeScene(State::Result);
    }
}

void SceneGame::updatePhysics() { m_world.step(static_cast<float>(Scene::DeltaTime())); }

void SceneGame::updateUI() { m_ui.update(Scene::DeltaTime()); }

void SceneGame::updateItems()
{
    // UIのクリック判定
    const bool uiClicked = m_ui.handleClick();

    // マウスカーソルから静的オブジェクトへのレイキャスト
    const Ray ray = m_camera.screenToRay(Cursor::Pos());
    m_raycastResult = m_world.raycast(ray, MASK_STATIC_ONLY);

    // Gravityアイテム更新
    updateGravityField();

    // Freezeアイテム更新
    updateFreezeField();

    // Windアイテム更新
    updateWindField();

    // アイテム投擲
    if (!uiClicked && MouseL.down() && m_raycastResult.hasHit && m_ui.canUseSelectedItem())
    {
        const ItemType selectedItem = m_ui.getSelectedItem();
        throwItem(selectedItem, m_raycastResult.hitPoint);
        m_ui.startReload(selectedItem);
    }
}

void SceneGame::updateGameObjects()
{
    // 全オブジェクトの状態更新
    for (const auto& object : m_gameObjects)
    {
        object->update();
    }

    // 全イベントの処理
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
    // EnemyNormal
    if (m_enemyNormalSpawnTimer.sF() >= m_normalSpawnInterval)
    {
        spawnEnemyNormal();
        m_enemyNormalSpawnTimer.restart();
    }

    // Explosive Enemy
    if (m_enemyExplosiveSpawnTimer.sF() >= m_explosiveSpawnInterval)
    {
        spawnEnemy();
        m_enemyExplosiveSpawnTimer.restart();
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

        // Freezeエフェクト
        // 氷柱
        if (!m_iceSpikes.isEmpty())
        {
            const ScopedRenderStates3D blend{BlendState::Additive};
            for (const auto& spike : m_iceSpikes)
            {
                const double t = Min(spike.timer.sF() / SpikeGrowDuration, 1.0);
                const double h = EaseOutBack(t) * spike.targetHeight;

                const Vec3 from = spike.position;
                const Vec3 to = spike.position + spike.direction * h;

                Cone{from, to, spike.radius}
                    .draw(FreezeEffectColor.withA(SpikeAlpha));
            }
        }
        // 範囲
        if (m_freezeField)
        {
            const Vec3 pos = m_freezeField->position + Vec3{0, FrostWaveOffsetY, 0};
            Cylinder{pos, FreezeRadius, FrostWaveHeight}.draw(m_frostTexture, ColorF{1.0, 1.0});
        }

        // 吸引範囲の可視化
        if (m_gravityField)
        {
            const ScopedRenderStates3D blend{BlendState::Additive};

            // 重力の中心
            const double pulseSize = Periodic::Sine0_1(1.5s) * 0.3 + 0.5;
            const Vec3 centerPos = m_gravityField->position + Vec3{0, 1.0, 0};
            Sphere{centerPos, pulseSize}.draw(GravityEffectColor.withA(0.8));

            const Vec3 pos = m_gravityField->position + Vec3{0, FrostWaveOffsetY, 0};
            Cylinder{pos, m_gravityField->radius, FrostWaveHeight}.draw(m_darknessTexture, ColorF{1.0, 1.0});
        }

        // wind範囲の可視化
        // 良いテクスチャが見つからなかったため、コメントアウト
        // if (m_windField)
        // {
        //     const Vec3 center = m_windField->area.center;
        //     const Vec3 size = m_windField->area.size;
        //     const Vec3 floorPos = Vec3{center.x, FrostWaveOffsetY, center.z};
        //     const Vec3 floorSize = Vec3{size.x, FrostWaveHeight, size.z};
        //     Box{floorPos, floorSize}.draw(m_windTexture, ColorF{1.0, 1.0});
        // }

        // アイテムの着地点と範囲の可視化
        if (m_raycastResult.hasHit)
        {
            const ItemType selectedItem = m_ui.getSelectedItem();
            const Vec3& targetPos = m_raycastResult.hitPoint;

            // インジケータのアルファ値を時間で変化させる
            const double alpha = Periodic::Sine0_1(IndicatorPulseDuration) * (IndicatorPulseMaxAlpha - IndicatorPulseMinAlpha) + IndicatorPulseMinAlpha;

            // 中心のマーカー
            Sphere{targetPos, 0.1}.draw(Palette::Red);

            const ScopedRenderStates3D blend{ BlendState::Additive };

            switch (selectedItem)
            {
                case ItemType::Bomb:
                {
                    Cylinder{targetPos, 5.0f, IndicatorHeight}.draw(BombIndicatorColor.withA(alpha));
                    break;
                }
                case ItemType::Gravity:
                {
                    Cylinder{targetPos, AttractionRadius, IndicatorHeight}.draw(GravityEffectColor.withA(alpha));
                    break;
                }
                case ItemType::Freeze:
                {
                    Cylinder{targetPos, FreezeRadius, IndicatorHeight}.draw(FreezeEffectColor.withA(alpha));
                    break;
                }
                case ItemType::Wind:
                {
                    Box{targetPos, Vec3{WindBoxWidth, IndicatorHeight, WindBoxDepth}}.draw(WindEffectColor.withA(alpha));
                    break;
                }
            }
        }
        // 10x10のグリッド、1マス1.0単位 （デバッグ）
        for (int i = -15; i <= 15; ++i)
        {
            s3d::Line3D({i, 1, -15}, {i, 1, 15}).draw(s3d::Palette::Gray);
            s3d::Line3D({-15, 1, i}, {15, 1, i}).draw(s3d::Palette::Gray);
        }

        // Bulletデバッグ描画（Dキーでトグル）
        m_world.debugDraw();
    }

    // [2D rendering]
    {
        // Flush 3D rendering commands before multisample resolve
        Graphics3D::Flush();

        // Multisample resolve
        m_renderTexture.resolve();

        // Transfer renderTexture to the current 2D scene (default scene)
        Shader::LinearToScreen(m_renderTexture);

        // UI を描画
        {
            m_instructionFont(U"D：デバッグ描画 [{}]"_fmt(m_debugDrawEnabled ? U"ON" : U"OFF"))
                .draw(30, 85, ColorF{1.0, 1.0, 1.0});
            m_instructionFont(U"T：タイトルへ戻る").draw(30, 115, ColorF{1.0, 1.0, 1.0});
        }

        // UIを描画
        m_ui.draw();
    }
}

void SceneGame::addGameObject(std::shared_ptr<GameObject> obj) { m_gameObjects.push_back(std::move(obj)); }

void SceneGame::createStage()
{
    // Stageオブジェクトを作成
    const auto stageParams = Stage::StageParams{.roadWidth = 25.0,
                                                .grassWidth = 100.0,
                                                .depth = 500.0,
                                                .position = Vec3{0, 0, 0},
                                                .restitution = 0.8f,
                                                .friction = 0.8f};

    auto stage = Stage::Create(m_world, stageParams);
    m_roadWidth = stageParams.roadWidth;

    // 木を配置 (Poisson Disk Sampling)
    const double minDistance = 15.0; // 木同士の最小距離 (密度を調整)
    const double offset = -15.0;     // 領域の端から内側へのオフセット

    // 左側の草原
    {
        const RectF leftGrassArea{-stageParams.roadWidth / 2 - stageParams.grassWidth, 0, stageParams.grassWidth,
                                  stageParams.depth};
        const RectF samplingArea = leftGrassArea.stretched(offset);
        s3d::PoissonDisk2D sampler(samplingArea.size.asPoint(), minDistance);
        const Array<Vec2> points = sampler.getPoints();
        for (const auto& p : points)
        {
            const Vec2 translatedPos = p + samplingArea.pos;
            const double scale = Random(2.5, 2.8);
            const double rot = Random(0.0, Math::TwoPi);
            stage->addTree(Vec3{translatedPos.x, 0, translatedPos.y}, scale, rot);
        }
    }

    // 右側の草原
    {
        const RectF rightGrassArea{stageParams.roadWidth / 2, 0, stageParams.grassWidth, stageParams.depth};
        const RectF samplingArea = rightGrassArea.stretched(offset);
        s3d::PoissonDisk2D sampler(samplingArea.size.asPoint(), minDistance);
        const Array<Vec2> points = sampler.getPoints();
        for (const auto& p : points)
        {
            const Vec2 translatedPos = p + samplingArea.pos;
            const double scale = Random(2.5, 2.8);
            const double rot = Random(0.0, Math::TwoPi);
            stage->addTree(Vec3{translatedPos.x, 0, translatedPos.y}, scale, rot);
        }
    }

    addGameObject(std::move(stage));
}

void SceneGame::spawnEnemy()
{
    // ステージ内のランダムな位置にスポーン
    const double offset = 1.0;
    const double x = Random(-m_roadWidth / 2.0 + offset, m_roadWidth / 2.0 - offset);
    const double z = Random(38.0, 42.0);
    const double y = 2.0;

    addGameObject(EnemyExplosive::Create(m_world,
                                         EnemyExplosive::EnemyExplosiveParams{.position = Vec3{x, y, z},
                                                                              .radius = 0.5f,
                                                                              .mass = 2.0f,
                                                                              .maxHealth = 50,
                                                                              .color = HSV{0, 0.7, 0.9},
                                                                              .group = GROUP_ATTRACTABLE,
                                                                              .mask = MASK_ALL,
                                                                              .explosionRadius = 3.0},
                                         m_enemyExplosiveModel));
}

void SceneGame::spawnEnemyNormal()
{

    // ステージ内のランダムな位置にスポーン
    const double offset = 1.0;
    const double x = Random(-m_roadWidth / 2.0 + offset, m_roadWidth / 2.0 - offset);
    const double z = Random(38.0, 42.0);
    const double y = 2.0;

    addGameObject(EnemyNormal::Create(m_world,
                                      EnemyNormal::EnemyNormalParams{.position = Vec3{x, y, z},
                                                                     .radius = 1.0f,
                                                                     .mass = 1.0f,
                                                                     .maxHealth = 50,
                                                                     .color = HSV{120, 0.7, 0.9},
                                                                     .group = GROUP_ATTRACTABLE,
                                                                     .mask = MASK_ALL},
                                      m_enemyNormalModel, U"LicensedAsset/normalEnemy.obj"));
}

void SceneGame::handleExplosion(const ExplosionRequest& request)
{
    // 爆発を実行（パーティクル + 物理的な力）
    createExplosionParticles(request.position, request.radius);
    applyExplosionForce(request);

    // サウンド再生
    m_explosionSound.playOneShot();

    // 画面揺れを開始
    shake(ExplosionShakeDuration, ExplosionShakeMagnitude);
}

void SceneGame::createExplosionParticles(const s3d::Vec3& center, double radius)
{
    s3d::Logger << U"   Creating {} particles"_fmt(ParticleCount);

    for (int32 i = 0; i < ParticleCount; ++i)
    {
        const double theta = s3d::Random(0.0, s3d::Math::TwoPi);
        const double phi = s3d::Random(0.0, s3d::Math::Pi);
        const double speed = s3d::Random(MinParticleSpeed, MaxParticleSpeed);

        s3d::Vec3 direction{s3d::Math::Sin(phi) * s3d::Math::Cos(theta), s3d::Math::Sin(phi) * s3d::Math::Sin(theta),
                            s3d::Math::Cos(phi)};

        Particle3D particle{.position = center,
                            .velocity = direction * speed,
                            .acceleration = Vec3{0, -5.0, 0},
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

    s3d::Logger << U"   Applying force to objects...";

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
        if (auto enemy = std::dynamic_pointer_cast<EnemyExplosive>(object))
        {
            int damage = static_cast<int>(falloff * 100);
            enemy->takeDamage(damage);
            Logger << U"  → Hit Enemy: distance {:.2f}, damage {}"_fmt(distance, damage);
        }
        else if (auto enemyNormal = std::dynamic_pointer_cast<EnemyNormal>(object))
        {
            int damage = static_cast<int>(falloff * 100);
            enemyNormal->takeDamage(damage);
            Logger << U"  → Hit EnemyNormal: distance {:.2f}, damage {}"_fmt(distance, damage);
        }
        else
        {
            Logger << U"  → Hit: distance {:.2f}, force {:.2f}"_fmt(distance, explosionForce);
        }
    }
    s3d::Logger << U"   Hit {} objects"_fmt(hitCount);
}

void SceneGame::createGravityParticles(const Vec3& center, double radius)
{
    // 3～5個のパーティクルを生成
    const int count = Random(3, 5);
    for (int i = 0; i < count; ++i)
    {
        // 円周上のランダムな点（水平方向のみ）
        const double angle = Random(0.0, Math::TwoPi);
        const Vec3 startPos = center + Vec3{
            Math::Cos(angle) * radius,
            Random(-0.3, 0.3), // わずかな高さのばらつき
            Math::Sin(angle) * radius
        };

        // 中心に向かう速度（主に水平方向）
        const Vec3 velocity = (center - startPos).normalized() * 2.5;
        const Vec3 acceleration = (center - startPos).normalized() * 5.0;

        m_particleSystem.add(Particle3D{
            .position = startPos,
            .velocity = velocity,
            .acceleration = acceleration,
            .color = ColorF{0.8, 0.4, 1.0}, // 紫色
            .size = Random(0.15, 0.25),
            .life = Random(1.0, 1.5),
            .active = true,
            .killZone = Sphere{center, 0.2}
        });
    }
}

void SceneGame::createFreezeParticles(const Vec3& center)
{
    for (int i = 0; i < FreezeParticleCount; ++i)
    {
        const Vec2 horizontal = RandomVec2(Circle{FreezeParticleHorizontalRadius}); // 横方向のランダム
        const Vec3 velocity{horizontal.x, Random(FreezeParticleMinSpeedY, FreezeParticleMaxSpeedY), horizontal.y};

        m_particleSystem.add(Particle3D{
            .position = center,
            .velocity = velocity,
            .acceleration = Vec3{0, -5.0, 0},
            .color = FreezeParticleColor,
            .size = Random(FreezeParticleMinSize, FreezeParticleMaxSize),
            .life = Random(FreezeParticleMinLife, FreezeParticleMaxLife),
            .active = true
        });
    }
}

void SceneGame::createWindParticles(const Vec3& center, const Vec3& boxSize)
{
    const int count = Random(5, 10);
    for (int i = 0; i < count; ++i)
    {
        const double x = Random(-boxSize.x / 2.0, boxSize.x / 2.0);
        const double y = Random(0.0, boxSize.y);
        const double z = -boxSize.z / 2.0;
        const Vec3 startPos = center + Vec3{x, y, z};

        const Vec3 velocity = Vec3{0, 0, Random(8.0, 12.0)};

        m_particleSystem.add(Particle3D{
            .position = startPos,
            .velocity = velocity,
            .acceleration = Vec3{0, 0, 0},
            .color = WindEffectColor,
            .size = Random(0.05, 0.15),
            .life = Random(2.0, 3.0),
            .active = true
        });
    }
}

void SceneGame::shake(double duration, double magnitude)
{
    m_shakeDuration = duration;
    m_shakeMagnitude = magnitude;
    m_shakeTimer.restart();
}

void SceneGame::throwBomb(const Vec3& targetPos)
{
    m_launchBombSound.playOneShot();

    const Vec3 startPos = m_camera.getEyePosition();
    constexpr double launchAngle = 10.0;
    const Vec3 gravity = m_world.getGravity();

    if (auto launchVelocity = PhysicsWorld::CalculateLaunchVelocity(startPos, targetPos, launchAngle, gravity))
    {
        const float mass = 2.0f;
        const float radius = 0.4f;

        Bomb::BombParams params{
            .position = startPos,
            .radius = radius,
            .mass = mass,
            .duration = 3.0,
            .color = ColorF{1.0, 0.5, 0.2},
            .restitution = 0.4f,
            .friction = 0.8f,
            .explosionRadius = 5.0f,
        };

        if (auto newBomb = Bomb::Create(m_world, params))
        {
            const Vec3 impulse = *launchVelocity * mass;
            newBomb->getPhysicsBody()->applyImpulse(impulse);
            addGameObject(std::move(newBomb));
        }
    }
    else
    {
        Logger << U"目標地点に到達できません";
    }
}

void SceneGame::throwGravity(const Vec3& targetPos)
{
    m_gravitySound.playOneShot();
    m_gravityField = GravityField{
        .position = targetPos,
        .remainingTime = GravityFieldDuration,
        .radius = AttractionRadius,
    };
    m_gravityParticleTimer.restart(); // タイマーを開始
}

void SceneGame::throwItem(ItemType itemType, const Vec3& targetPos)
{
    switch (itemType)
    {
    case ItemType::Bomb:
        throwBomb(targetPos);
        break;
    case ItemType::Gravity:
        throwGravity(targetPos);
        break;
    case ItemType::Freeze:
        throwFreeze(targetPos);
        break;
    case ItemType::Wind:
        throwWind(targetPos);
        break;
    }
}

void SceneGame::updateGravityField()
{
    if (!m_gravityField)
    {
        m_gravityParticleTimer.reset(); // タイマーをリセット
        return;
    }

    m_gravityField->remainingTime -= Scene::DeltaTime();

    if (m_gravityField->remainingTime <= 0.0)
    {
        m_gravityField.reset();
        return;
    }

    applyGravityFieldForce();

    // パーティクルの定期生成
    if (m_gravityParticleTimer.sF() >= 0.05) // 0.05秒ごと
    {
        const Vec3 centerPos = m_gravityField->position + Vec3{0, 1.0, 0};

        createGravityParticles(centerPos, m_gravityField->radius);
        m_gravityParticleTimer.restart();
    }
}

void SceneGame::applyGravityFieldForce()
{
    const Vec3& hitPoint = m_gravityField->position;

    for (const auto& object : m_gameObjects)
    {
        if (auto body = object->getPhysicsBody(); body && body->getGroup() == GROUP_ATTRACTABLE)
        {
            const Vec3 objPos = object->getPosition();
            const Vec3 direction = (hitPoint - objPos);
            const double distanceSq = direction.lengthSq();

            if (distanceSq < (m_gravityField->radius * m_gravityField->radius))
            {
                const Vec3 force = direction.normalized() * AttractionForce;
                body->applyForce(force);
            }
        }
    }
}

void SceneGame::throwFreeze(const Vec3& targetPos)
{
    m_freezeSound.playOneShot();
    createFreezeParticles(targetPos);

    m_freezeField = FreezeField{
        .position = targetPos,
        .remainingTime = FreezeDuration,
        .radius = FreezeRadius,
    };

    // 氷柱をランダム生成
    const int spikeCount = Random(MinSpikeCount, MaxSpikeCount);
    for (int i = 0; i < spikeCount; ++i)
    {
        // 範囲内のランダム位置
        const Vec2 offset = RandomVec2(Circle(FreezeRadius * 0.8));
        const Vec3 direction = Vec3{Random(-SpikeDirectionRandomness, SpikeDirectionRandomness),
                                    1.0,
                                    Random(-SpikeDirectionRandomness, SpikeDirectionRandomness)}.normalized();

        m_iceSpikes.push_back(IceSpike{
            .position = targetPos + Vec3{offset.x, 0, offset.y},
            .direction = direction,
            .targetHeight = Random(MinSpikeHeight, MaxSpikeHeight),
            .radius = Random(MinSpikeRadius, MaxSpikeRadius),
            .timer = Stopwatch{StartImmediately::Yes}
        });
    }
}

void SceneGame::updateFreezeField()
{
    if (!m_freezeField)
    {
        m_iceSpikes.clear(); // Freeze終了時に氷柱をクリア
        return;
    }

    m_freezeField->remainingTime -= Scene::DeltaTime();

    if (m_freezeField->remainingTime <= 0.0)
    {
        m_freezeField.reset();
        return;
    }

    applyFreezeEffect();
}

void SceneGame::applyFreezeEffect()
{
    const Vec3& centerPos = m_freezeField->position;

    for (const auto& object : m_gameObjects)
    {
        if (auto body = object->getPhysicsBody();
            body && body->getGroup() == GROUP_ATTRACTABLE)
        {
            const Vec3 objPos = object->getPosition();
            const double distanceSq = (centerPos - objPos).lengthSq();

            if (distanceSq < (m_freezeField->radius * m_freezeField->radius))
            {
                body->setLinearVelocity(Vec3::Zero());
                body->setAngularVelocity(Vec3::Zero());
            }
        }
    }
}

void SceneGame::throwWind(const Vec3& targetPos)
{
    m_windSound.playOneShot();
    const Vec3 boxCenter = Vec3{targetPos.x, WindBoxHeight / 2.0, targetPos.z};
    const Vec3 boxSize = Vec3{WindBoxWidth, WindBoxHeight, WindBoxDepth};

    m_windField = WindField{
        .area = Box{boxCenter, boxSize},
        .force = Vec3{0, 0, WindForce},
        .remainingTime = WindDuration,
    };
    m_windParticleTimer.restart();
}

void SceneGame::updateWindField()
{
    if (!m_windField)
    {
        m_windParticleTimer.reset();
        return;
    }

    m_windField->remainingTime -= Scene::DeltaTime();

    if (m_windField->remainingTime <= 0.0)
    {
        m_windField.reset();
        return;
    }

    applyWindEffect();

    // パーティクルの定期生成
    if (m_windParticleTimer.sF() >= 0.05)
    {
        createWindParticles(m_windField->area.center, m_windField->area.size);
        m_windParticleTimer.restart();
    }
}

void SceneGame::applyWindEffect()
{
    const Vec3 windForce = m_windField->force;

    for (const auto& object : m_gameObjects)
    {
        if (auto body = object->getPhysicsBody();
            body && body->getGroup() == GROUP_ATTRACTABLE)
        {
            if (m_windField->area.contains(object->getPosition()))
            {
                body->applyForce(windForce);
            }
        }
    }
}
