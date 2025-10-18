#pragma once
#include "SceneGame.h"
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
class SceneTestExplosion : public SceneGame
{
public:
    SceneTestExplosion(const InitData& init);

    void update() override;
    void draw() const override;

private:
    // 爆発関数
    void explode(GameObject* bomb, double radius);

    // 爆弾オブジェクトのID
    GameObject::IDType m_bombID = 0;

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
