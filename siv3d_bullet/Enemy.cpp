#include "Enemy.h"
#include "PhysicsWorld.h"
#include "ExplosionHelper.h"

Enemy::Enemy(std::unique_ptr<PhysicsBody> physicsBody, std::unique_ptr<IRenderer> renderer, int maxHealth, double explosionRadius)
    : GameObject(std::move(physicsBody), std::move(renderer)), m_health(maxHealth), m_maxHealth(maxHealth), m_explosionRadius(explosionRadius)
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

    auto enemy = std::make_shared<Enemy>(std::move(body), std::move(renderer), params.maxHealth, params.explosionRadius);
    enemy->getPhysicsBody()->setOwner(enemy->weak_from_this());
    return enemy;
}

void Enemy::update()
{
    // タイマーの更新はStopwatchが自動的に行う
}

bool Enemy::isReadyToExplode() const
{
    return (m_state == State::Dying && m_deathTimer.sF() >= 1.0);
}

void Enemy::triggerExplosion(ParticleSystem& particleSystem, const s3d::Array<std::shared_ptr<GameObject>>& gameObjects)
{
    if (m_state != State::Dying)
    {
        return;
    }

    ExplosionHelper::CreateExplosion(particleSystem, gameObjects, getPosition(), m_explosionRadius, shared_from_this());

    m_state = State::Dead;
}
