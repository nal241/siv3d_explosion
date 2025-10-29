#include "EnemyNormal.h"
#include "PhysicsWorld.h"

EnemyNormal::EnemyNormal(std::unique_ptr<PhysicsBody> physicsBody, std::unique_ptr<IRenderer> renderer, int maxHealth)
    : GameObject(std::move(physicsBody), std::move(renderer)), m_health(maxHealth), m_maxHealth(maxHealth)
{
}

void EnemyNormal::takeDamage(int damage)
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

std::shared_ptr<EnemyNormal> EnemyNormal::Create(PhysicsWorld& world, const EnemyNormalParams& params,
                                                 const s3d::Model& model, const s3d::FilePath& modelPath)
{
    // コンパウンドシェイプ（楕円体+円錐）を作成
    const float ellipsoidRadiusX = params.radius * 0.48;
    const float ellipsoidRadiusY = params.radius * 0.36f;
    const float ellipsoidRadiusZ = params.radius * 0.48;
    const float coneRadius = params.radius * 0.4f;
    const float coneHeight = params.radius * 0.3f;

    auto body = world.createCompoundShape()
                    .addEllipsoid({0, ellipsoidRadiusY, 0}, {ellipsoidRadiusX, ellipsoidRadiusY, ellipsoidRadiusZ})
                    .addCone({0, ellipsoidRadiusY * 2, 0}, coneRadius, coneHeight)
                    .build(params.position, params.mass, params.group, params.mask);

    body->setRestitution(params.restitution);
    body->setFriction(params.friction);
    body->setDamping(0.1f, 0.8f);

    auto renderer = std::make_unique<ModelRenderer>(model);

    auto enemy = std::make_shared<EnemyNormal>(std::move(body), std::move(renderer), params.maxHealth);
    enemy->getPhysicsBody()->setOwner(enemy->weak_from_this());
    return enemy;
}

void EnemyNormal::update()
{
    // 死亡状態のタイマー更新
    if (m_state == State::Dying && m_deathTimer.sF() >= 1.0)
    {
        m_state = State::Dead;
    }
}
