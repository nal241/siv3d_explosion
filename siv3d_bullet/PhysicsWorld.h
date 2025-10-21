#pragma once

#include <btBulletDynamicsCommon.h>

#include "CollisionGroups.h"
#include "PhysicsBody.h"

// 前方宣言
class GameObject;

struct RaycastResult
{
    bool hasHit = false;
    // NOTE: hitObjectはweak_ptrで保持。使用時にlock()して有効性を確認すること。
    std::weak_ptr<GameObject> hitObject;
    s3d::Vec3 hitPoint;
    s3d::Vec3 hitNormal;
};

class PhysicsWorld
{
public:
    PhysicsWorld();
    ~PhysicsWorld();

    // 削除の禁止
    PhysicsWorld(const PhysicsWorld&) = delete;
    PhysicsWorld& operator=(const PhysicsWorld&) = delete;

    // シミュレーションを1ステップ進める
    void step(float deltaTime);

    RaycastResult raycast(const s3d::Ray& ray, CollisionMask mask = MASK_ALL, double maxDistance = 1000.0);

    // 重力取得
    s3d::Vec3 getGravity() const;

    // オブジェクト追加（unique_ptrで返す）
    std::unique_ptr<PhysicsBody> createBox(const BoxDesc& desc, CollisionGroup group, CollisionMask mask);
    std::unique_ptr<PhysicsBody> createSphere(const SphereDesc& desc, CollisionGroup group, CollisionMask mask);
    std::unique_ptr<PhysicsBody> createCylinder(const CylinderDesc& desc, CollisionGroup group, CollisionMask mask);

    // --- static utilities ---
    static std::optional<Vec3> CalculateLaunchVelocity(const Vec3& start, const Vec3& target, double launchAngleDeg,
                                                       const Vec3& gravity);

private:
    friend class PhysicsBody;

    // Bulletのコアコンポーネント
    std::unique_ptr<btDefaultCollisionConfiguration> m_collisionConfig;
    std::unique_ptr<btCollisionDispatcher> m_dispatcher;
    std::unique_ptr<btDbvtBroadphase> m_broadphase;
    std::unique_ptr<btSequentialImpulseConstraintSolver> m_solver;
    std::unique_ptr<btDiscreteDynamicsWorld> m_dynamicsWorld;

    // 作成したオブジェクトを管理
    s3d::HashSet<PhysicsBody*> m_registeredObjects;

    // PhysicsObjectからの通知メソッド
    void registerObject(PhysicsBody* obj);
    void unregisterObject(PhysicsBody* obj);
};
