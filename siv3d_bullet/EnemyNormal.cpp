#include "EnemyNormal.h"
#include "PhysicsWorld.h"

namespace
{
    // --- 形状パラメータ ---
    constexpr float SHAPE_ELLIPSOID_RATIO_X = 0.48f; // 楕円体のX軸比率
    constexpr float SHAPE_ELLIPSOID_RATIO_Y = 0.36f; // 楕円体のY軸比率
    constexpr float SHAPE_ELLIPSOID_RATIO_Z = 0.48f; // 楕円体のZ軸比率
    constexpr float SHAPE_CONE_RATIO_RADIUS = 0.4f;  // 円錐の半径比率
    constexpr float SHAPE_CONE_RATIO_HEIGHT = 0.3f;  // 円錐の高さ比率
    constexpr float SHAPE_CONE_Y_OFFSET = 2.0f;      // 円錐のY軸オフセット倍率

    // --- 物理パラメータ ---
    constexpr float LINEAR_DAMPING = 0.2f;                        // 線形減衰
    constexpr float ANGULAR_DAMPING = 0.8f;                       // 角速度減衰
    constexpr Vec3 ANGULAR_FACTOR_LOCKED = Vec3{0.5, 0.0, 0.5};   // 回転制限あり
    constexpr Vec3 ANGULAR_FACTOR_UNLOCKED = Vec3{1.0, 1.0, 1.0}; // 回転制限なし

    // --- ジャンプパラメータ ---
    constexpr Vec3 JUMP_IMPULSE = Vec3{0, 3.0, -1.8}; // ジャンプのインパルス（Y: 上方向, Z: 前方向）
    constexpr Vec3 TARGET_FORWARD = Vec3{0, 0, -1};   // ジャンプ時の目標方向
    constexpr Vec3 WORLD_UP = Vec3{0, 1, 0};          // ワールドの上方向

    // --- その他 ---
    constexpr double DEATH_DURATION = 3.0; // 死亡状態の継続時間（秒）
} // namespace

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
        getPhysicsBody()->setAngularFactor(ANGULAR_FACTOR_UNLOCKED);
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
                                                 const s3d::Model& model)
{
    // コンパウンドシェイプ（楕円体+円錐）を作成
    const float ellipsoidRadiusX = params.radius * SHAPE_ELLIPSOID_RATIO_X;
    const float ellipsoidRadiusY = params.radius * SHAPE_ELLIPSOID_RATIO_Y;
    const float ellipsoidRadiusZ = params.radius * SHAPE_ELLIPSOID_RATIO_Z;
    const float coneRadius = params.radius * SHAPE_CONE_RATIO_RADIUS;
    const float coneHeight = params.radius * SHAPE_CONE_RATIO_HEIGHT;

    auto body = world.createCompoundShape()
                    .addEllipsoid({0, ellipsoidRadiusY, 0}, {ellipsoidRadiusX, ellipsoidRadiusY, ellipsoidRadiusZ})
                    .addCone({0, ellipsoidRadiusY * SHAPE_CONE_Y_OFFSET, 0}, coneRadius, coneHeight)
                    .build(params.position, params.mass, params.group, params.mask);

    body->setRestitution(params.restitution);
    body->setFriction(params.friction);
    body->setDamping(LINEAR_DAMPING, ANGULAR_DAMPING);

    // 回転を抑制
    body->setAngularFactor(ANGULAR_FACTOR_LOCKED);

    // 半径に応じてモデルのスケールを調整（基準半径は0.5とする）
    const double scale = params.radius / 0.5;
    auto renderer = std::make_unique<ModelRenderer>(model, Palette::White, scale);

    auto enemy = std::make_shared<EnemyNormal>(std::move(body), std::move(renderer), params.maxHealth);
    enemy->getPhysicsBody()->setOwner(enemy->weak_from_this());
    return enemy;
}

bool EnemyNormal::isStable() const
{
    // 姿勢チェック: 上方向ベクトルがワールドY軸とどれだけ一致しているか
    const Quaternion rot = getPhysicsBody()->getRotation();
    const Vec3 localUp = rot * WORLD_UP;
    const double dotProduct = localUp.dot(WORLD_UP);
    const double angleThreshold = Math::Cos(Math::ToRadians(STABLE_ANGLE_THRESHOLD));

    // 角速度チェック: 回転が落ち着いているか
    const Vec3 angularVel = getPhysicsBody()->getAngularVelocity();
    const double angularSpeed = angularVel.length();

    // 線速度チェック: 全体の速度がほぼゼロ（静止している）
    const Vec3 linearVel = getPhysicsBody()->getLinearVelocity();
    const double totalSpeed = linearVel.length();

    return (dotProduct > angleThreshold) && (angularSpeed < STABLE_ANGULAR_VELOCITY_THRESHOLD) &&
           (totalSpeed < STABLE_LINEAR_VELOCITY_THRESHOLD);
}

void EnemyNormal::update()
{
    // 死亡状態のタイマー更新
    if (m_state == State::Dying && m_deathTimer.sF() >= DEATH_DURATION)
    {
        m_state = State::Dead;
    }

    // 生存中、姿勢・速度が安定している場合のみジャンプ
    if (m_state == State::Alive && m_jumpTimer.sF() >= m_jumpInterval && isStable())
    {
        // ジャンプ前に向きを正面（Z軸負の方向）にリセット
        // TODO: より自然な動きにするには、徐々に回転させる処理に変更する
        // 注: Y軸回転のみをリセットし、X/Z軸の傾きは保持する簡易実装
        const Quaternion currentRot = getPhysicsBody()->getRotation();
        const Vec3 currentUp = currentRot * WORLD_UP;
        const Vec3 right = TARGET_FORWARD.cross(currentUp).normalized();
        const Vec3 forward = currentUp.cross(right).normalized();
        const Quaternion correctedRot = Quaternion::FromUnitVectors(TARGET_FORWARD, forward);
        getPhysicsBody()->setRotation(correctedRot);

        // 前方（Z軸の負の方向）と上方向にインパルスを加える
        getPhysicsBody()->applyImpulse(JUMP_IMPULSE);

        // タイマーをリセット
        m_jumpTimer.restart();
    }
}
