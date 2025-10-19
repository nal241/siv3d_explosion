#pragma once
#include "GameObject.h"

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
    };

    Enemy(std::unique_ptr<PhysicsBody> physicsBody, std::unique_ptr<IRenderer> renderer, int maxHealth);

    void takeDamage(int damage);
    bool shouldBeRemoved() const override { return m_health <= 0; }
    int getHealth() const { return m_health; }
    int getMaxHealth() const { return m_maxHealth; }

    static std::shared_ptr<Enemy> Create(PhysicsWorld& world, const EnemyParams& params);

private:
    int m_health;
    int m_maxHealth;
};
