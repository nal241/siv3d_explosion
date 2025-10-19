#pragma once
#include "IComponent.h"

class ExplosionComponent : public IComponent
{
public:
    ExplosionComponent() = default;

    // 爆発タイマーを起動する
    void activate(double delay, double radius);

    // 毎フレームの更新（IComponentからオーバーライド）
    void update() override;

    // 爆発が起動中か
    bool isPending() const;

    // 設定された爆発半径を取得する
    double getRadius() const;

    // 爆発した瞬間か
    bool justExploded() const;

    // 爆発イベントを消費する（フラグをリセット）
    void consumeExplosion();

private:
    bool m_isPending = false;
    bool m_justExploded = false; // 追加
    s3d::Stopwatch m_timer;
    double m_delay = 0.0;
    double m_radius = 0.0;
};