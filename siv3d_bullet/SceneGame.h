#pragma once
#include "SceneCommon.h"
#include "PhysicsWorld.h"
#include "Player.h"
#include "ParticleSystem.h"
#include "UI.h"

// ゲームシーン
class SceneGame : public App::Scene
{
public:
    SceneGame(const InitData& init);

    void update() override;

    void draw() const override;

protected:
    // 更新処理を機能ごとに分割
    virtual void updateInput();
    virtual void updateUI();
    virtual void updateGameLogic();
    virtual void updatePhysics();
    virtual void updateAudio();
    virtual void updateCamera();

    // updateGameLogic内で呼ばれる内部メソッド
    void updateItems();
    void updateGameObjects();
    void updateSpawn();
    void updateCombo();

    // updateAudio内で呼ばれる内部メソッド
    void updateExplosionSound();

    // その他の更新メソッド
    void updateParticleSystem();
    // ヘルパーメソッド
    void removeObjects();
    void createStage();
    void addGameObject(std::shared_ptr<GameObject> obj);
    void spawnEnemy();
    void spawnEnemyNormal();
    void handleExplosion(const ExplosionRequest& request);

    // アイテム投擲
    void throwBomb(const Vec3& targetPos);
    void throwGravity(const Vec3& targetPos);

    void throwFreeze(const Vec3& targetPos);
    void throwWind(const Vec3& targetPos);
    void throwItem(ItemType itemType, const Vec3& targetPos);

    // 重力アイテム
    void updateGravityField();
    void applyGravityFieldForce();

    // Freezeアイテム
    void updateFreezeField();
    void applyFreezeEffect();
    void unfreezeObject(std::shared_ptr<GameObject> obj);

    // Windアイテム用
    void updateWindField();
    void applyWindEffect();

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
    ColorF m_backgroundColor = ColorF{0.5, 0.7, 0.9}.removeSRGBCurve();

    const MSRenderTexture m_renderTexture;
    // BasicCamera3D m_camera;
    DebugCamera3D m_camera;
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

    s3d::Model m_enemyNormalModel;
    s3d::Model m_enemyExplosiveModel;

    Player m_player;

    // エネミースポーン用
    Stopwatch m_enemyNormalSpawnTimer{StartImmediately::Yes};
    double m_normalSpawnInterval = 0.5; // normalEnemyは高頻度
    Stopwatch m_enemyExplosiveSpawnTimer{StartImmediately::Yes};
    double m_explosiveSpawnInterval = 1.0; // Enemyは低頻度

    double m_spawnInterval = 3.0;
    double m_roadWidth = 0.0;

    ParticleSystem m_particleSystem;
    s3d::Audio m_explosionSound{U"example/explosion1.mp3"};
    s3d::Audio m_launchBombSound;
    s3d::Audio m_gravitySound;
    s3d::Audio m_freezeSound;
    s3d::Audio m_windSound;
    s3d::Audio m_bgm;

    // 爆発音の管理（うねり防止）
    Stopwatch m_explosionSoundTimer{StartImmediately::Yes};
    int m_explosionCountInInterval = 0;     // 間隔内の爆発回数
    double m_explosionSoundInterval = 0.05; // 音再生の最小間隔（秒）

    // UI用フォント
    s3d::Font m_titleFont{40, s3d::Typeface::Bold};
    s3d::Font m_instructionFont{24};

    // デバッグ描画の有効/無効
    bool m_debugDrawEnabled = true;

    // UI
    UI m_ui;

    // Gravityアイテム
    struct GravityField
    {
        Vec3 position;
        double remainingTime;
        double radius;
    };
    s3d::Optional<GravityField> m_gravityField;

    // Freezeアイテム
    struct FreezeField
    {
        Vec3 position;
        double remainingTime;
        double radius;
    };
    s3d::Optional<FreezeField> m_freezeField;
    s3d::Array<std::weak_ptr<GameObject>> m_frozenObjects;

    // Windアイテム用
    struct WindField
    {
        s3d::Box area;
        Vec3 force;
        double remainingTime;
    };
    s3d::Optional<WindField> m_windField;

    // コンボシステム
    int m_comboCount = 0;           // 現在のコンボ数
    int m_maxCombo = 0;             // 最大コンボ数
    int m_comboScore = 0;           // コンボ期間中の総スコア
    double m_comboTimeWindow = 2.0; // コンボ継続判定時間（秒）
    Stopwatch m_comboTimer{StartImmediately::No};
    void incrementCombo();             // コンボをカウントアップ
    void resetCombo();                 // コンボをリセット
    double getComboMultiplier() const; // コンボ倍率を取得
};
