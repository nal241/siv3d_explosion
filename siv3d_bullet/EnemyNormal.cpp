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

    // 回転を抑制
    body->setAngularFactor(Vec3{0.5, 0.0, 0.5});

    auto renderer = std::make_unique<ModelRenderer>(model);

    auto enemy = std::make_shared<EnemyNormal>(std::move(body), std::move(renderer), params.maxHealth);
    enemy->getPhysicsBody()->setOwner(enemy->weak_from_this());
    return enemy;
}

bool EnemyNormal::isStable() const
{
    // 姿勢チェック: 上方向ベクトルがワールドY軸とどれだけ一致しているか
    const Quaternion rot = getPhysicsBody()->getRotation();
    const Vec3 localUp = rot * Vec3{0, 1, 0};
    const double dotProduct = localUp.dot(Vec3{0, 1, 0});
    const double angleThreshold = Math::Cos(Math::ToRadians(STABLE_ANGLE_THRESHOLD));

    // 角速度チェック: 回転が落ち着いているか
    const Vec3 angularVel = getPhysicsBody()->getAngularVelocity();
    const double angularSpeed = angularVel.length();

    return (dotProduct > angleThreshold) && (angularSpeed < STABLE_ANGULAR_VELOCITY_THRESHOLD);
}

void EnemyNormal::update()
{
    // 死亡状態のタイマー更新
    if (m_state == State::Dying && m_deathTimer.sF() >= 1.0)
    {
        m_state = State::Dead;
    }

    // 生存中かつ姿勢が安定している場合のみジャンプ
    if (m_state == State::Alive && m_jumpTimer.sF() >= m_jumpInterval && isStable())
    {
        // ジャンプ前に向きを正面（Z軸負の方向）にリセット
        // TODO: より自然な動きにするには、徐々に回転させる処理に変更する
        // 注: Y軸回転のみをリセットし、X/Z軸の傾きは保持する簡易実装
        const Quaternion currentRot = getPhysicsBody()->getRotation();
        const Vec3 currentUp = currentRot * Vec3{0, 1, 0};
        const Vec3 targetForward = Vec3{0, 0, -1};
        const Vec3 right = targetForward.cross(currentUp).normalized();
        const Vec3 forward = currentUp.cross(right).normalized();
        const Quaternion correctedRot = Quaternion::FromUnitVectors(Vec3{0, 0, -1}, forward);
        getPhysicsBody()->setRotation(correctedRot);

        // 前方（Z軸の負の方向）と上方向にインパルスを加える
        const Vec3 jumpImpulse{0, 5.0, -3.0}; // Y: 上方向, Z: 前方向
        getPhysicsBody()->applyImpulse(jumpImpulse);

        // タイマーをリセット
        m_jumpTimer.restart();
    }
}
