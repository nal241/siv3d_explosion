#pragma once

#include <btBulletDynamicsCommon.h>
#include "PhysicsObject.h"

class PhysicsWorld {
public:
    // コンストラクタで初期化
    PhysicsWorld();
    // デストラクタで後片付け
    ~PhysicsWorld();

    // シミュレーションを1ステップ進める
    void step(float deltaTime);

    // オブジェクト追加（Siv3D型・スマートポインタで返す）
    std::unique_ptr<PhysicsObject> addBox(const BoxDesc& desc);
    std::unique_ptr<PhysicsObject> addSphere(const SphereDesc& desc);

private:
    // Bulletのコアコンポーネント
    btDefaultCollisionConfiguration* m_collisionConfiguration;
    btCollisionDispatcher* m_dispatcher;
    btBroadphaseInterface* m_broadphase;
    btSequentialImpulseConstraintSolver* m_solver;
    btDiscreteDynamicsWorld* m_dynamicsWorld;

    // 作成したオブジェクトを管理するためのリスト
    std::vector<btCollisionShape*> m_collisionShapes;
    std::vector<btRigidBody*> m_rigidBodies;

    void addObject(PhysicsObject& obj);
};
