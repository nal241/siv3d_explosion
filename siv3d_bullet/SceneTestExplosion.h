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
    // 3D空間のパーティクルシステム
    s3d::Array<Particle3D> m_particles;
    static constexpr s3d::Vec3 Gravity{0, -5.0, 0};

    // 効果音
    s3d::Audio m_explosionSound{U"example/explosion1.mp3"};
    // 背景色
    s3d::ColorF m_backgroundColor = s3d::ColorF{0.8, 0.3, 0.2}.removeSRGBCurve();
    // UI用フォント
    s3d::Font m_titleFont{40, s3d::Typeface::Bold};
    s3d::Font m_instructionFont{24};
    s3d::Font m_cooldownFont{16, s3d::Typeface::Bold};

    // 爆弾投擲のクールダウンタイマー
    s3d::Stopwatch m_throwCooldown;

    // レイキャストの結果
    RaycastResult m_raycastResult;
};