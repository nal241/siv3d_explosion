#include "Enemy.h"
#include "PhysicsWorld.h"

Enemy::Enemy(std::unique_ptr<PhysicsBody> physicsBody, std::unique_ptr<IRenderer> renderer, int maxHealth)
    : GameObject(std::move(physicsBody), std::move(renderer)), m_health(maxHealth), m_maxHealth(maxHealth)
{
}

void Enemy::takeDamage(int damage)
{
    m_health -= damage;
    if (m_health < 0)
    {
        m_health = 0;
    }
}

std::shared_ptr<Enemy> Enemy::Create(PhysicsWorld& world, const EnemyParams& params)
{
    CollisionGroup group = params.group;
    CollisionMask mask = params.mask;

    auto body = world.createSphere(SphereDesc{params.radius, params.position, params.mass}, group, mask);
    body->setRestitution(params.restitution);
    body->setFriction(params.friction);

    auto renderer = std::make_unique<PhysicsShapeRenderer>(*body, params.color);

    auto enemy = std::make_shared<Enemy>(std::move(body), std::move(renderer), params.maxHealth);
    enemy->getPhysicsBody()->setOwner(enemy->weak_from_this());
    return enemy;
}
