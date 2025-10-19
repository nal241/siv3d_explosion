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

private:
    bool m_isPending = false;
    s3d::Stopwatch m_timer;
    double m_delay = 0.0;
    double m_radius = 0.0;
};