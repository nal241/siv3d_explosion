#pragma once
#include "SceneCommon.h"
#include "PhysicsWorld.h"
#include "Player.h"
#include "ParticleSystem.h"

// ゲームシーン
class SceneGame : public App::Scene
{
public:
    SceneGame(const InitData& init);

    void update() override;

    void draw() const override;

protected:
    // 更新処理を機能ごとに分割
    virtual void updateCamera();
    virtual void updateInput();
    virtual void updatePhysics();
    virtual void updateGameObjects();
    virtual void updateParticleSystem();
    virtual void updateSpawn();
    virtual void updateSceneSpecific() {}

    // ヘルパーメソッド
    void removeObjects();
    void createStage();
    void addGameObject(std::shared_ptr<GameObject> obj);
    void spawnEnemy();
    void spawnEnemyNormal();
    void handleExplosion(const ExplosionRequest& request);

    // 爆発処理
    void createExplosionParticles(const s3d::Vec3& center, double radius);
    void applyExplosionForce(const ExplosionRequest& request);

protected:
    // 画面揺れを開始する
    void shake(double duration, double magnitude);

    // メンバ変数
    PhysicsWorld m_world;

    // GameObjectの所有権を持つコンテナ
    // NOTE: 子クラスが特定のオブジェクトへの参照を保持する場合は、
    //       weak_ptrまたは生ポインタを使用するとよい。
    //       shared_ptrでの二重所有はさける。
    s3d::Array<std::shared_ptr<GameObject>> m_gameObjects;

    RaycastResult m_raycastResult;

    // Background color (remove SRGB curve for a linear workflow)
    ColorF m_backgroundColor = ColorF{0.4, 0.6, 0.8}.removeSRGBCurve();

    const MSRenderTexture m_renderTexture;
    BasicCamera3D m_camera;
    s3d::Vec3 m_cameraPosition;
    s3d::Vec3 m_cameraLookAt;

    // 画面揺れ用
    Stopwatch m_shakeTimer{StartImmediately::No};
    double m_shakeDuration = 0.0;
    double m_shakeMagnitude = 0.0;
    s3d::PerlinNoise m_shakeNoise;
    double m_shakeNoiseTime = 0.0;
    s3d::Vec3 m_noiseSeeds;

    Texture m_uvChecker{U"example/texture/uv.png", TextureDesc::MippedSRGB};
    Model m_model{U"model/coin.obj"};
    s3d::Model m_enemyNormalModel;
    s3d::Model m_explosiveEnemyModel;

    Player m_player;

    // エネミースポーン用
    Stopwatch m_enemyNormalSpawnTimer{StartImmediately::Yes};
    double m_normalSpawnInterval = 0.5; // normalEnemyは高頻度
    Stopwatch m_explosiveEnemySpawnTimer{StartImmediately::Yes};
    double m_explosiveSpawnInterval = 4.0; // Enemyは低頻度

    ParticleSystem m_particleSystem;
    s3d::Audio m_explosionSound{U"example/explosion1.mp3"};
};
