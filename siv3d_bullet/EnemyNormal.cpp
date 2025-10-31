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
        Logger << U"[EnemyNormal ID:" << getID() << U"] ダメージ無視: 既に死亡状態";
        return;
    }

    // 爆発を受けた時、回転制限を解除
    if (m_rotationLocked)
    {
        m_rotationLocked = false;
        getPhysicsBody()->setAngularFactor(Vec3{1.0, 1.0, 1.0}); // すべての軸で回転可能に
        Logger << U"[EnemyNormal ID:" << getID() << U"] y軸回転の制限を解除";
    }

    const int prevHealth = m_health;
    m_health -= damage;
    Logger << U"[EnemyNormal ID:" << getID() << U"] ダメージ受けた: " << damage << U" (HP: " << prevHealth << U" -> "
           << m_health << U")";

    if (m_health <= 0)
    {
        m_health = 0;
        m_state = State::Dying;
        m_deathTimer.start();
        Logger << U"[EnemyNormal ID:" << getID() << U"] 死亡状態に移行";
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

    // y軸回転を禁止（x軸とz軸は許可）
    body->setAngularFactor(Vec3{1.0, 0.0, 1.0});

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

    // 生存中のみジャンプ
    if (m_state == State::Alive && m_jumpTimer.sF() >= m_jumpInterval)
    {
        // 前方（Z軸の負の方向）と上方向にインパルスを加える
        const Vec3 jumpImpulse{0, 5.0, -3.0}; // Y: 上方向, Z: 前方向
        getPhysicsBody()->applyImpulse(jumpImpulse);

        // タイマーをリセット
        m_jumpTimer.restart();
    }
}
