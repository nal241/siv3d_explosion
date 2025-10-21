#include "Bomb.h"
#include "PhysicsWorld.h"
#include "Renderers.h"

Bomb::Bomb(std::unique_ptr<PhysicsBody> physicsBody, std::unique_ptr<IRenderer> renderer, double duration,
           double explosionRadius)
    : GameObject(std::move(physicsBody), std::move(renderer)), m_duration(duration), m_explosionRadius(explosionRadius)
{
    m_timer.start();
}

void Bomb::update()
{
    // タイマーの更新はStopwatchが自動的に行う
}

bool Bomb::shouldBeRemoved() const { return m_isExploded; }

bool Bomb::isReadyToExplode() const { return m_timer.sF() >= m_duration && !m_isExploded; }

void Bomb::triggerExplosion(ParticleSystem& particleSystem, const s3d::Array<std::shared_ptr<GameObject>>& gameObjects)
{
    if (m_isExploded)
    {
        return;
    }

    ExplosionHelper::CreateExplosion(particleSystem, gameObjects, getPosition(), m_explosionRadius, shared_from_this());
    m_isExploded = true;
}

std::shared_ptr<Bomb> Bomb::Create(PhysicsWorld& world, const BombParams& params)
{
    auto body = world.createSphere(SphereDesc{params.radius, params.position, params.mass}, params.group, params.mask);
    body->setRestitution(params.restitution);
    body->setFriction(params.friction);

    auto renderer = std::make_unique<PhysicsShapeRenderer>(*body, params.color);

    auto bomb = std::make_shared<Bomb>(std::move(body), std::move(renderer), params.duration, params.explosionRadius);
    bomb->getPhysicsBody()->setOwner(bomb->weak_from_this());
    return bomb;
}
