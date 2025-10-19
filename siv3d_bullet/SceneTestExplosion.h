#pragma once
#include "SceneGame.h"
#include "GameObject.h"
#include "PhysicsWorld.h"
#include "ExplosionHelper.h" // ★ Particle3D構造体をインクルード

class SceneTestExplosion : public SceneGame
{
public:
    SceneTestExplosion(const InitData& init);

    void draw() const override;

protected:
    void updateSceneSpecific() override;

private:
    // 爆発関数
    void explode(const std::shared_ptr<GameObject>& bomb, double radius);

    // 爆弾オブジェクトへの弱参照
    std::weak_ptr<GameObject> m_bombObject;

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
