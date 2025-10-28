#include "ExplosiveEnemy.h"
#include "PhysicsWorld.h"
#include "Renderers.h"

ExplosiveEnemy::ExplosiveEnemy(std::unique_ptr<PhysicsBody> physicsBody, std::unique_ptr<IRenderer> renderer,
                               int maxHealth, double explosionRadius)
    : GameObject(std::move(physicsBody), std::move(renderer)), m_health(maxHealth), m_maxHealth(maxHealth),
      m_explosionRadius(explosionRadius)
{
}

void ExplosiveEnemy::takeDamage(int damage)
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

std::shared_ptr<ExplosiveEnemy> ExplosiveEnemy::Create(PhysicsWorld& world, const ExplosiveEnemyParams& params,
                                                       const s3d::Model& model)
{
    CollisionGroup group = params.group;
    CollisionMask mask = params.mask;

    auto body = world.createSphere(SphereDesc{params.radius, params.position, params.mass}, group, mask);
    body->setRestitution(params.restitution);
    body->setFriction(params.friction);
    // 回転しないように
    body->setAngularFactor({0., 0., 0.});
    // 浮遊している
    body->setGravity({0., 0., 0.});

    // body->setDamping(0.2f, 0.1f);

    auto renderer = std::make_unique<ModelRenderer>(model);

    auto enemy = std::make_shared<ExplosiveEnemy>(std::move(body), std::move(renderer), params.maxHealth,
                                                  params.explosionRadius);
    enemy->getPhysicsBody()->setOwner(enemy->weak_from_this());
    enemy->m_initialX = params.position.x;
    return enemy;
}

void ExplosiveEnemy::applyPDControl()
{
    auto currentPosition = getPosition();
    auto currentVelocity = getPhysicsBody()->getLinearVelocity();

    // 高さの制御
    auto currentAltitude = currentPosition.y;
    auto error = targetAltitude - currentAltitude;

    auto verticalVelocity = currentVelocity.y;

    float hoverForceY = (error * hoverKpY) - (verticalVelocity * hoverKdY);

    // 左右

    auto currentX = currentPosition.x;
    auto errorX = m_initialX - currentX;

    auto horizontalVelocity = currentVelocity.x;

    float moveForceX = (errorX * moveKpX) - (horizontalVelocity * moveKdX);

    // 進行速度
    auto currentSpeed = currentVelocity.z;
    auto errorSpeed = targetSpeed - currentSpeed;

    float moveForceZ = (errorSpeed * moveKpZ);

    getPhysicsBody()->applyForce({moveForceX, hoverForceY, moveForceZ});
}

void ExplosiveEnemy::update()
{
    // タイマーの更新はStopwatchが自動的に行う
    if (isReadyToExplode())
    {
        // 爆発イベントを発行（物理的な力の適用、パーティクル、サウンド）
        emitEvent(ExplosionRequest{getPosition(), m_explosionRadius, weak_from_this()});

        m_state = State::Dead;
    }

    // 位置制御
    applyPDControl();
}
