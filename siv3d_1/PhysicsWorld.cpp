#include "PhysicsWorld.h"

// コンストラクタ：ワールドのセットアップを行う
PhysicsWorld::PhysicsWorld() {
	m_collisionConfiguration = new btDefaultCollisionConfiguration();
	m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
	m_broadphase = new btDbvtBroadphase();
	m_solver = new btSequentialImpulseConstraintSolver();
	m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);

	// 重力を設定
	m_dynamicsWorld->setGravity(btVector3(0, -9.81, 0));
	m_dynamicsWorld->getSolverInfo().m_numIterations = 20;
}

// デストラクタ：確保した全てのリソースを解放する
PhysicsWorld::~PhysicsWorld() {
	// 1. 剛体をワールドから削除し、メモリを解放
	for (int i = m_rigidBodies.size() - 1; i >= 0; i--) {
		btRigidBody* body = m_rigidBodies[i];
		m_dynamicsWorld->removeRigidBody(body);
		// btRigidBodyをdeleteすると、関連するbtMotionStateも自動でdeleteされる
		delete body;
	}
	m_rigidBodies.clear();

	// 2. 衝突形状のメモリを解放
	for (int i = m_collisionShapes.size() - 1; i >= 0; i--) {
		delete m_collisionShapes[i];
	}
	m_collisionShapes.clear();

	// 3. ワールドのコアコンポーネントを解放 (作成と逆順)
	delete m_dynamicsWorld;
	delete m_solver;
	delete m_broadphase;
	delete m_dispatcher;
	delete m_collisionConfiguration;
}

// シミュレーションを進める
void PhysicsWorld::step(float deltaTime) {
	m_dynamicsWorld->stepSimulation(deltaTime, 10);
}

// 箱を追加する
btRigidBody* PhysicsWorld::addBox(const btVector3& size, const btVector3& position, float mass) {
	btCollisionShape* shape = new btBoxShape(size);
	m_collisionShapes.push_back(shape); // 解放漏れがないようにリストに保持

	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(position);
	btDefaultMotionState* motionState = new btDefaultMotionState(transform);

	btVector3 localInertia(0, 0, 0);
	if (mass != 0.0f) {
		shape->calculateLocalInertia(mass, localInertia);
	}

	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, localInertia);
	btRigidBody* body = new btRigidBody(rbInfo);

	m_dynamicsWorld->addRigidBody(body);
	m_rigidBodies.push_back(body); // 解放漏れがないようにリストに保持

	return body;
}

// 球を追加する
btRigidBody* PhysicsWorld::addSphere(float radius, const btVector3& position, float mass) {
	btCollisionShape* shape = new btSphereShape(radius);
	m_collisionShapes.push_back(shape);

	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(position);
	btDefaultMotionState* motionState = new btDefaultMotionState(transform);

	btVector3 localInertia(0, 0, 0);
	if (mass != 0.0f) {
		shape->calculateLocalInertia(mass, localInertia);
	}

	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, localInertia);
	btRigidBody* body = new btRigidBody(rbInfo);

	m_dynamicsWorld->addRigidBody(body);
	m_rigidBodies.push_back(body);

	return body;
}
