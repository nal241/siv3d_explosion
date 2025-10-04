#include "PhysicsWorld.h"
#include "PhysicsObject.h"
#include "BulletSiv3dUtils.h"

// コンストラクタ：ワールドのセットアップを行う
PhysicsWorld::PhysicsWorld()
	: m_collisionConfig(std::make_unique<btDefaultCollisionConfiguration>()), m_dispatcher(std::make_unique<btCollisionDispatcher>(m_collisionConfig.get())), m_broadphase(std::make_unique<btDbvtBroadphase>()), m_solver(std::make_unique<btSequentialImpulseConstraintSolver>()), m_dynamicsWorld(std::make_unique<btDiscreteDynamicsWorld>(
																																																																						 m_dispatcher.get(), m_broadphase.get(), m_solver.get(), m_collisionConfig.get()))
{
	m_dynamicsWorld->setGravity(btVector3(0, -9.81, 0));
	m_dynamicsWorld->getSolverInfo().m_numIterations = 20;
}

// デストラクタ：確保した全てのリソースを解放する
PhysicsWorld::~PhysicsWorld()
{
	// 登録されているオブジェクトに親の削除を通知
	for (auto *obj : m_registeredObjects)
	{
		obj->m_world = nullptr;
	}
}

// シミュレーションを進める
void PhysicsWorld::step(float deltaTime)
{
	m_dynamicsWorld->stepSimulation(deltaTime, 10);
}

// 箱を作成する
std::unique_ptr<PhysicsObject> PhysicsWorld::createBox(const BoxDesc &desc)
{
	auto shape = std::make_unique<btBoxShape>(ToBtVector3(desc.size * 0.5));
	return std::unique_ptr<PhysicsObject>(
		new PhysicsObject(this, std::move(shape), desc.mass, desc.position));
}

// 球を作成する
std::unique_ptr<PhysicsObject> PhysicsWorld::createSphere(const SphereDesc &desc)
{
	auto shape = std::make_unique<btSphereShape>(desc.radius);
	return std::unique_ptr<PhysicsObject>(
		new PhysicsObject(this, std::move(shape), desc.mass, desc.position));
}

void PhysicsWorld::registerObject(PhysicsObject *obj)
{
	m_registeredObjects.insert(obj);
	m_dynamicsWorld->addRigidBody(obj->m_body.get());
}

void PhysicsWorld::unregisterObject(PhysicsObject *obj)
{
	if (m_registeredObjects.erase(obj) > 0)
	{
		if (m_dynamicsWorld)
		{
			m_dynamicsWorld->removeRigidBody(obj->m_body.get());
		}
	}
}
