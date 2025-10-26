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

std::shared_ptr<EnemyNormal> EnemyNormal::Create(PhysicsWorld& world, const EnemyNormalParams& params, const s3d::Model& model, const s3d::FilePath& modelPath)
{
    // モデルパスからConvex Hullを作成
    // NOTE: OBJファイルのスケールをそろえるため2倍に調整
    // NOTE: Z軸を反転（前後が逆だったため）
    const double scaleMultiplier = 2.0;
    auto body = world.createConvexHull(
        ConvexHullDesc{&model, modelPath, params.position, params.mass, Vec3{params.radius * scaleMultiplier, params.radius * scaleMultiplier, -params.radius * scaleMultiplier}},
        params.group, params.mask);
    body->setRestitution(params.restitution);
    body->setFriction(params.friction);

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
