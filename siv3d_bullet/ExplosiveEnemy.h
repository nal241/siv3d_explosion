#pragma once
#include "GameObject.h"
#include "ParticleSystem.h"

class ExplosiveEnemy : public GameObject
{
public:
    struct ExplosiveEnemyParams
    {
        Vec3 position;
        float radius = 0.5f;
        float mass = 2.0f;
        int maxHealth = 50;
        ColorF color = HSV{0, 0.7, 0.9};
        float restitution = 0.3f;
        float friction = 0.5f;
        CollisionGroup group = GROUP_DEFAULT;
        CollisionMask mask = MASK_ALL;
        double explosionRadius = 3.0;
    };

    ExplosiveEnemy(std::unique_ptr<PhysicsBody> physicsBody, std::unique_ptr<IRenderer> renderer, int maxHealth,
                   double explosionRadius);

    void update() override;
    void takeDamage(int damage);
    bool shouldBeRemoved() const override { return m_state == State::Dead; }
    int getHealth() const { return m_health; }
    int getMaxHealth() const { return m_maxHealth; }

    static std::shared_ptr<ExplosiveEnemy> Create(PhysicsWorld& world, const ExplosiveEnemyParams& params,
                                                  const s3d::Model& model);

private:
    enum class State
    {
        Alive,
        Dying,
        Dead
    };

    bool isReadyToExplode() const { return m_state == State::Dying && m_deathTimer.sF() >= 1.0; }

    int m_health;
    int m_maxHealth;
    double m_explosionRadius;
    State m_state = State::Alive;
    Stopwatch m_deathTimer;

    // 位置の制御
    const double targetAltitude = 1.5;
    const double targetSpeed = -2.0;
    double m_initialX = 0.0;

    const double hoverKpY = 10.0;
    const double hoverKdY = 0.3;

    const double moveKpX = 1.0;
    const double moveKdX = 0.1;

    const double moveKpZ = 2.0;

    void applyPDControl();
};
