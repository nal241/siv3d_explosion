#pragma once
#include "GameObject.h"

class EnemyNormal : public GameObject
{
public:
    struct EnemyNormalParams
    {
        Vec3 position;
        float radius = 0.5f;
        float mass = 1.0f;
        int maxHealth = 50;
        ColorF color = HSV{120, 0.7, 0.9};
        float restitution = 0.3f;
        float friction = 0.5f;
        CollisionGroup group = GROUP_ATTRACTABLE;
        CollisionMask mask = MASK_ALL;
    };

    EnemyNormal(std::unique_ptr<PhysicsBody> physicsBody, std::unique_ptr<IRenderer> renderer, int maxHealth);

    void update() override;
    void takeDamage(int damage);
    bool shouldBeRemoved() const override { return m_state == State::Dead; }
    int getHealth() const { return m_health; }
    int getMaxHealth() const { return m_maxHealth; }

    static std::shared_ptr<EnemyNormal> Create(PhysicsWorld& world, const EnemyNormalParams& params,
                                               const s3d::Model& model, const s3d::FilePath& modelPath);

private:
    enum class State
    {
        Alive,
        Dying,
        Dead
    };

    int m_health;
    int m_maxHealth;
    State m_state = State::Alive;
    Stopwatch m_deathTimer;

    // ジャンプ用
    Stopwatch m_jumpTimer{StartImmediately::Yes};
    double m_jumpInterval = 1.5; // ジャンプ間隔（秒）

    // 回転制限フラグ
    bool m_rotationLocked = true; // 初期状態ではy軸回転を禁止
};
