#include "Enemy.h"
#include "PhysicsWorld.h"

Enemy::Enemy(std::unique_ptr<PhysicsBody> physicsBody, std::unique_ptr<IRenderer> renderer, int maxHealth,
             double explosionRadius)
    : GameObject(std::move(physicsBody), std::move(renderer)), m_health(maxHealth), m_maxHealth(maxHealth),
      m_explosionRadius(explosionRadius)
{
}

void Enemy::takeDamage(int damage)
{
    if (m_state != State::Alive)
    {
        return;
    }

    m_health -= damage;
    if (m_health <= 0)
    {
        m_health = 0;
        m_state = State::Dying;
        m_deathTimer.start();
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

    auto enemy =
        std::make_shared<Enemy>(std::move(body), std::move(renderer), params.maxHealth, params.explosionRadius);
    enemy->getPhysicsBody()->setOwner(enemy->weak_from_this());
    return enemy;
}

void Enemy::update()
{
    // タイマーの更新はStopwatchが自動的に行う

    if (isReadyToExplode())
    {
        // 爆発イベントを発行（物理的な力の適用、パーティクル、サウンド）
        emitEvent(ExplosionRequest{getPosition(), m_explosionRadius, weak_from_this()});

        m_state = State::Dead;
    }
}

