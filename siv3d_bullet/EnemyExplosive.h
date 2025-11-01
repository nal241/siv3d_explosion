#pragma once
#include "GameObject.h"
#include "ParticleSystem.h"

class EnemyExplosive : public GameObject
{
public:
    struct EnemyExplosiveParams
    {
        static constexpr float radius = 0.8f;
        static constexpr float mass = 1.0f;
        static constexpr int maxHealth = 50;
        static constexpr float restitution = 0.3f;
        static constexpr float friction = 0.5f;

        Vec3 position;
        double explosionRadius = 3.0;
        ColorF color = HSV{0, 0.7, 0.9};
        CollisionGroup group = GROUP_DEFAULT;
        CollisionMask mask = MASK_ALL;
    };

    EnemyExplosive(std::unique_ptr<PhysicsBody> physicsBody, std::unique_ptr<IRenderer> renderer, int maxHealth,
                   double explosionRadius);

    void update() override;

    void draw() const override;
    void takeDamage(int damage);
    bool shouldBeRemoved() const override { return m_state == State::Dead; }
    bool isAlive() const { return m_state == State::Alive; }
    int getHealth() const { return m_health; }
    int getMaxHealth() const { return m_maxHealth; }
    double getBlinkIntensity() const;
    static std::shared_ptr<EnemyExplosive> Create(PhysicsWorld& world, const EnemyExplosiveParams& params,
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
    static constexpr double targetAltitude = 1.5;
    static constexpr double targetSpeed = -2.0;
    double m_initialX = 0.0;

    // PD制御ゲイン（質量1.0あたりの基準値 × 質量5.0）
    static constexpr double hoverKpY = 5.0 * EnemyExplosiveParams::mass;
    static constexpr double hoverKdY = 0.15 * EnemyExplosiveParams::mass;
    static constexpr double moveKpX = 0.5 * EnemyExplosiveParams::mass;
    static constexpr double moveKdX = 0.05 * EnemyExplosiveParams::mass;
    static constexpr double moveKpZ = 1.0 * EnemyExplosiveParams::mass;

    void applyPDControl();
};
