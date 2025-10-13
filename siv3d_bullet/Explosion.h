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
    // 爆発関数
    void explode(PhysicsObject* bomb, double radius);

    // 物理エンジン
    PhysicsWorld m_world;
    HashTable<PhysicsObject::IDType, std::unique_ptr<PhysicsObject>> m_physicsObjects;

    // 爆弾オブジェクト（特別に管理）
    PhysicsObject* m_bomb = nullptr; // ポインタで保持（配列内のオブジェクトを参照）
    

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

	// ★ 効果音
    Audio m_explosionSound{U"example/explosion1.mp3"};
};
