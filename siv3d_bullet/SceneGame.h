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
    // ヘルパーメソッド
    void removeOutOfBoundsObjects();
    void createStage();
    std::unique_ptr<GameObject> createStaticBox(const Vec3& size, const Vec3& position, const ColorF& color);
    void addGameObject(std::unique_ptr<GameObject> obj);

    // メンバ変数
    PhysicsWorld m_world;
    HashTable<GameObject::IDType, std::unique_ptr<GameObject>> m_gameObjects;

    // Background color (remove SRGB curve for a linear workflow)
    ColorF m_backgroundColor = ColorF{0.4, 0.6, 0.8}.removeSRGBCurve();

    const MSRenderTexture m_renderTexture;
    DebugCamera3D m_camera;

    Texture m_uvChecker{U"example/texture/uv.png", TextureDesc::MippedSRGB};
    Model m_model{U"model/coin.obj"};

    Player m_player;
};