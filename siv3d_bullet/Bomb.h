#pragma once
#include "GameObject.h"

class Bomb : public GameObject
{
public:
    struct BombParams
    {
        Vec3 position;
        float radius = 0.5f;
        float mass = 1.0f;
        double duration = 2.0; // 爆発までの時間
        ColorF color = Palette::Black;
        float restitution = 0.3f;
        float friction = 0.5f;
        CollisionGroup group = GROUP_DEFAULT;
        CollisionMask mask = MASK_ALL;
        double explosionRadius = 5.0;
    };

    Bomb(std::unique_ptr<PhysicsBody> physicsBody, std::unique_ptr<IRenderer> renderer, double duration,
         double explosionRadius);

    void update() override;
    bool shouldBeRemoved() const override;

    static std::shared_ptr<Bomb> Create(PhysicsWorld& world, const BombParams& params);

private:
    bool isReadyToExplode() const { return m_timer.sF() >= m_duration && !m_isExploded; }
    Stopwatch m_timer;
    double m_duration;
    bool m_isExploded = false;
    double m_explosionRadius;
};
