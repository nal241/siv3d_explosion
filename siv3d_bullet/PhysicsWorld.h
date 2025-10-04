#pragma once

#include <btBulletDynamicsCommon.h>

#include "PhysicsObject.h"

class PhysicsWorld
{
public:
    PhysicsWorld();
    ~PhysicsWorld();

    // 削除禁止
    PhysicsWorld(const PhysicsWorld &) = delete;
    PhysicsWorld &operator=(const PhysicsWorld &) = delete;

    // シミュレーションを1ステップ進める
    void step(float deltaTime);

    // オブジェクト追加（unique_ptrで返す）
    std::unique_ptr<PhysicsObject> createBox(const BoxDesc &desc);
    std::unique_ptr<PhysicsObject> createSphere(const SphereDesc &desc);

private:
    friend class PhysicsObject;

    // Bulletのコアコンポーネント
    std::unique_ptr<btDefaultCollisionConfiguration> m_collisionConfig;
    std::unique_ptr<btCollisionDispatcher> m_dispatcher;
    std::unique_ptr<btDbvtBroadphase> m_broadphase;
    std::unique_ptr<btSequentialImpulseConstraintSolver> m_solver;
    std::unique_ptr<btDiscreteDynamicsWorld> m_dynamicsWorld;

    // 作成したオブジェクトを管理
    s3d::HashSet<PhysicsObject *> m_registeredObjects;

    // PhysicsObjectからの通知メソッド
    void registerObject(PhysicsObject *obj);
    void unregisterObject(PhysicsObject *obj);
};
