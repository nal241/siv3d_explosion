#pragma once
#include "SceneCommon.h"
#include "PhysicsWorld.h"
#include "Player.h"
// ★ 3D空間のパーティクル構造体
struct Particle3D
{
    Vec3 position;
    Vec3 velocity;
    ColorF color;
    double size;
    double life; // 残り寿命（0.0 ～ 1.0）
    bool active;
};
class Explosion : public App::Scene
{
public:
    Explosion(const InitData& init);
    void update() override;
    void draw() const override;

private:
    // 爆発関数
    void explode(GameObject* bomb, double radius);
    // 物理エンジン
    PhysicsWorld m_world;
    HashTable<GameObject::IDType, std::unique_ptr<GameObject>> m_gameObjects;
    // 爆弾オブジェクト
    GameObject* m_bomb = nullptr;
    // 描画用
    MSRenderTexture m_renderTexture;
    DebugCamera3D m_camera;
    // プレイヤー
    Player m_player;
    Model m_model{U"model/coin.obj"};
    Texture m_uvChecker{U"example/texture/uv.png", TextureDesc::MippedSRGB};
    // 3D空間のパーティクルシステム
    Array<Particle3D> m_particles;
    static constexpr Vec3 Gravity{0, -5.0, 0};
    // 効果音
    Audio m_explosionSound{U"example/explosion1.mp3"};
    // 背景色
    ColorF m_backgroundColor = ColorF{0.8, 0.3, 0.2}.removeSRGBCurve();
    // UI用フォント
    Font m_titleFont{40, Typeface::Bold};
    Font m_instructionFont{24};
};
