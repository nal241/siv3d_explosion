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

    if (isReadyToExplode())
    {
        // 爆発イベントを発行（物理的な力の適用、パーティクル、サウンドはSceneが処理）
        emitEvent(ExplosionRequest{getPosition(), m_explosionRadius, weak_from_this()});

        m_isExploded = true;
    }
}

bool Bomb::shouldBeRemoved() const { return m_isExploded; }

void Bomb::notifyCollision() { m_hasCollided = true; }

std::shared_ptr<Bomb> Bomb::Create(PhysicsWorld& world, const BombParams& params)
{
    auto body = world.createSphere(SphereDesc{params.radius, params.position, params.mass}, params.group, params.mask);
    body->setRestitution(params.restitution);
    body->setFriction(params.friction);

    auto renderer = std::make_unique<PhysicsShapeRenderer>(*body, params.color);

    auto bomb = std::make_shared<Bomb>(std::move(body), std::move(renderer), params.duration, params.explosionRadius);
    bomb->getPhysicsBody()->setOwner(bomb->weak_from_this());

    // 衝突コールバックを登録
    bomb->getPhysicsBody()->setCollisionCallback(
        [bombWeak = std::weak_ptr<Bomb>(bomb)]()
        {
            if (auto bombPtr = bombWeak.lock())
            {
                bombPtr->notifyCollision();
            }
        });

    return bomb;
}
