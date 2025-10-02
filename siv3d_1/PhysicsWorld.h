#pragma once

#include "btBulletDynamicsCommon.h"
class PhysicsWorld {
public:
	// コンストラクタで初期化
	PhysicsWorld();
	// デストラクタで後片付け
	~PhysicsWorld();

	// シミュレーションを1ステップ進める
	void step(float deltaTime);

	// オブジェクトを追加するヘルパー関数
	btRigidBody* addBox(const btVector3& size, const btVector3& position, float mass);
	btRigidBody* addSphere(float radius, const btVector3& position, float mass);

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
};
