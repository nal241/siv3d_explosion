#pragma once
#include "SceneCommon.h"
#include "PhysicsWorld.h"
#include "Player.h"

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
    virtual void updateSpawn();
    virtual void updateSceneSpecific() {}

    // ヘルパーメソッド
    void removeObjects();
    void createStage();
    void addGameObject(std::shared_ptr<GameObject> obj);
    void spawnEnemy();

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
    DebugCamera3D m_camera;

    Texture m_uvChecker{U"example/texture/uv.png", TextureDesc::MippedSRGB};
    Model m_model{U"model/coin.obj"};

    Player m_player;

    // エネミースポーン用
    Stopwatch m_enemySpawnTimer{StartImmediately::Yes};
    double m_spawnInterval = 3.0;
};
