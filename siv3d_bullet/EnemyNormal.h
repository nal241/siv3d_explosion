#pragma once
#include "GameObject.h"

class EnemyNormal : public GameObject
{
public:
    struct EnemyNormalParams
    {
        static constexpr float radius = 0.8f;
        static constexpr float mass = 0.5f;
        static constexpr int maxHealth = 50;
        static constexpr float restitution = 0.3f;
        static constexpr float friction = 0.8f;

        Vec3 position;
        ColorF color = HSV{120, 0.7, 0.9};
        CollisionGroup group = GROUP_ATTRACTABLE;
        CollisionMask mask = MASK_ALL;
    };

    EnemyNormal(std::unique_ptr<PhysicsBody> physicsBody, std::unique_ptr<IRenderer> renderer, int maxHealth);

    void update() override;
    void takeDamage(int damage);
    bool shouldBeRemoved() const override { return m_state == State::Dead; }
    bool isAlive() const { return m_state == State::Alive; }
    int getHealth() const { return m_health; }
    int getMaxHealth() const { return m_maxHealth; }

    static std::shared_ptr<EnemyNormal> Create(PhysicsWorld& world, const EnemyNormalParams& params,
                                               const s3d::Model& model);

private:
    enum class State
    {
        Alive,
        Dying,
        Dead
    };

    // --- 体力関連 ---
    int m_health;
    int m_maxHealth;
    State m_state = State::Alive;
    Stopwatch m_deathTimer;

    // --- ジャンプ関連 ---
    Stopwatch m_jumpTimer{StartImmediately::Yes};
    double m_jumpInterval = 1.5;  // ジャンプ間隔（秒）
    bool m_rotationLocked = true; // 初期状態ではy軸回転を禁止

    // --- 安定判定関連 ---
    bool isStable() const;                                           // 姿勢が安定しているか判定（角度+角速度+線速度）
    static constexpr double STABLE_ANGLE_THRESHOLD = 10.0;           // 安定とみなす角度閾値（度）
    static constexpr double STABLE_ANGULAR_VELOCITY_THRESHOLD = 0.1; // 安定とみなす角速度閾値（rad/s）
    static constexpr double STABLE_LINEAR_VELOCITY_THRESHOLD = 0.5;  // 安定とみなす速度閾値（m/s）
};
