#include "EnemyExplosive.h"
#include "PhysicsWorld.h"
#include "Renderers.h"

EnemyExplosive::EnemyExplosive(std::unique_ptr<PhysicsBody> physicsBody, std::unique_ptr<IRenderer> renderer,
                               int maxHealth, double explosionRadius)
    : GameObject(std::move(physicsBody), std::move(renderer)), m_health(maxHealth), m_maxHealth(maxHealth),
      m_explosionRadius(explosionRadius)
{
}

void EnemyExplosive::takeDamage(int damage)
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

std::shared_ptr<EnemyExplosive> EnemyExplosive::Create(PhysicsWorld& world, const EnemyExplosiveParams& params,
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

    // 半径に応じてモデルのスケールを調整
    const double scale = params.radius / 0.5;
    auto renderer = std::make_unique<ModelRenderer>(model, Palette::White, scale);

    auto enemy = std::make_shared<EnemyExplosive>(std::move(body), std::move(renderer), params.maxHealth,
                                                  params.explosionRadius);
    enemy->getPhysicsBody()->setOwner(enemy->weak_from_this());
    enemy->m_initialX = params.position.x;
    return enemy;
}

void EnemyExplosive::applyPDControl()
{
    auto currentPosition = getPosition();
    auto currentVelocity = getPhysicsBody()->getLinearVelocity();

    // 高さの制御
    auto currentAltitude = currentPosition.y;
    auto error = targetAltitude - currentAltitude;

    auto verticalVelocity = currentVelocity.y;

    double hoverForceY = (error * hoverKpY) - (verticalVelocity * hoverKdY);

    // 左右
    auto currentX = currentPosition.x;
    auto errorX = m_initialX - currentX;

    auto horizontalVelocity = currentVelocity.x;

    double moveForceX = (errorX * moveKpX) - (horizontalVelocity * moveKdX);

    // 進行速度
    auto currentSpeed = currentVelocity.z;
    auto errorSpeed = targetSpeed - currentSpeed;

    double moveForceZ = (errorSpeed * moveKpZ);

    getPhysicsBody()->applyForce({moveForceX, hoverForceY, moveForceZ});
}

void EnemyExplosive::update()
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
