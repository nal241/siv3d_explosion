#include "PhysicsWorld.h"
#include "PhysicsObject.h"

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
	for (size_t i = m_rigidBodies.size(); i-- > 0;) {
		btRigidBody* body = m_rigidBodies[i];
		m_dynamicsWorld->removeRigidBody(body);
		// btRigidBodyをdeleteすると、関連するbtMotionStateも自動でdeleteされる
		delete body->getCollisionShape();
		delete body;
	}
	m_rigidBodies.clear();

	// 2. 衝突形状のメモリを解放
	//for (size_t i = m_collisionShapes.size(); i-- > 0;) {
	//	delete m_collisionShapes[i];
	//}
	//m_collisionShapes.clear();

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
std::unique_ptr<PhysicsObject> PhysicsWorld::addBox(const BoxDesc& desc)
{
    auto obj = std::unique_ptr<PhysicsObject>(new PhysicsObject(desc));
    m_dynamicsWorld->addRigidBody(obj->getRigidBody());
    m_rigidBodies.push_back(obj->getRigidBody());
    return obj;
}

// 球を追加する
std::unique_ptr<PhysicsObject> PhysicsWorld::addSphere(const SphereDesc& desc)
{
    auto obj = std::unique_ptr<PhysicsObject>(new PhysicsObject(desc));
    m_dynamicsWorld->addRigidBody(obj->getRigidBody());
    m_rigidBodies.push_back(obj->getRigidBody());
    return obj;
}
