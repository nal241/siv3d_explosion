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

private:
    PhysicsWorld m_world;
    Array<std::unique_ptr<PhysicsObject>> m_physicsObjects;

    // Background color (remove SRGB curve for a linear workflow)
    ColorF m_backgroundColor = ColorF{0.4, 0.6, 0.8}.removeSRGBCurve();

    const MSRenderTexture m_renderTexture;
    DebugCamera3D m_camera;

    Texture m_uvChecker{U"example/texture/uv.png", TextureDesc::MippedSRGB};
    Model m_model{U"model/coin.obj"};

    Player m_player;

    void createStage();
};