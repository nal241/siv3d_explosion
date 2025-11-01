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

void Bomb::draw() const
{
    // 通常描画（PhysicsShapeRendererで球体描画）
    GameObject::draw();

    // 点滅時は発光球を追加
    const double intensity = getBlinkIntensity();
    if (intensity > 0.01)
    {
        const ScopedRenderStates3D blend{BlendState::Additive};
        PhongMaterial phong;
        phong.ambientColor = ColorF{0.0};
        phong.diffuseColor = ColorF{0.0};
        phong.emissionColor = ColorF{1.0, 0.3, 0.1}.removeSRGBCurve() * intensity * 2.0;

        Sphere{getPosition(), 0.5}.draw(phong);
    }
}

bool Bomb::shouldBeRemoved() const { return m_isExploded; }

void Bomb::notifyCollision() { m_hasCollided = true; }

double Bomb::getBlinkIntensity() const
{
    const double elapsed = m_timer.sF();
    const double progress = elapsed / m_duration;

    // 爆発が近づくほど点滅速度を上げる
    const double frequency = Math::Lerp(1.0, 2.0, progress);
    const double cycle = Math::Fmod(elapsed * frequency, 1.0);

    // 周期の30%は光る、70%は消灯
    return (cycle < 0.3) ? 1.0 : 0.0;
}

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
