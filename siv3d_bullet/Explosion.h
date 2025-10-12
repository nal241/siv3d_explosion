#pragma once
#include "SceneCommon.h"

#include "PhysicsWorld.h"
#include "Player.h"
class Explosion : public App::Scene
{
public:
    Explosion(const InitData& init);

    void update() override;

    void draw() const override;

private:
	// 物理エンジン
    PhysicsWorld m_world;
    Array<std::unique_ptr<PhysicsObject>> m_physicsObjects;

    // 背景色
	// Gameと違う色を設定
    ColorF m_backgroundColor = ColorF{0.8, 0.3, 0.2}.removeSRGBCurve();

	// 描画用
    const MSRenderTexture m_renderTexture;
    DebugCamera3D m_camera;

	// UI用フォント
    Font m_titleFont{40, Typeface::Bold};
    Font m_instructionFont{24};

	// Player
	Player m_player;

	Texture m_uvChecker{U"example/texture/uv.png", TextureDesc::MippedSRGB};
    Model m_model{U"model/coin.obj"};
};
