#pragma once
#include "GameObject.h"
#include "ParticleSystem.h"

class Enemy : public GameObject
{
public:
    struct EnemyParams
    {
        Vec3 position;
        float radius = 0.5f;
        float mass = 2.0f;
        int maxHealth = 100;
        ColorF color = HSV{0, 0.7, 0.9};
        float restitution = 0.3f;
        float friction = 0.5f;
        CollisionGroup group = GROUP_DEFAULT;
        CollisionMask mask = MASK_ALL;
        double explosionRadius = 3.0;
    };

    Enemy(std::unique_ptr<PhysicsBody> physicsBody, std::unique_ptr<IRenderer> renderer, int maxHealth,
          double explosionRadius);

    void update() override;
    void takeDamage(int damage);
    bool shouldBeRemoved() const override { return m_state == State::Dead; }
    int getHealth() const { return m_health; }
    int getMaxHealth() const { return m_maxHealth; }

    bool isReadyToExplode() const;
    void triggerExplosion(ParticleSystem& particleSystem, const s3d::Array<std::shared_ptr<GameObject>>& gameObjects);

    bool handleExplosionCheck(ParticleSystem& particleSystem,
                              const s3d::Array<std::shared_ptr<GameObject>>& gameObjects,
                              s3d::Audio& explosionSound) override;

    static std::shared_ptr<Enemy> Create(PhysicsWorld& world, const EnemyParams& params);

private:
    enum class State
    {
        Alive,
        Dying,
        Dead
    };

    int m_health;
    int m_maxHealth;
    double m_explosionRadius;
    State m_state = State::Alive;
    Stopwatch m_deathTimer;
};
