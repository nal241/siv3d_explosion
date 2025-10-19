#include "PhysicsWorld.h"
#include "BulletSiv3dUtils.h"
#include "GameObject.h"
#include "PhysicsBody.h"

namespace
{
    constexpr double GravityY = -9.81;
    constexpr int SolverIterations = 10;
    constexpr int MaxSubSteps = 5;
} // namespace

// コンストラクタ：ワールドのセットアップを行う
PhysicsWorld::PhysicsWorld()
    : m_collisionConfig(std::make_unique<btDefaultCollisionConfiguration>()),
      m_dispatcher(std::make_unique<btCollisionDispatcher>(m_collisionConfig.get())),
      m_broadphase(std::make_unique<btDbvtBroadphase>()),
      m_solver(std::make_unique<btSequentialImpulseConstraintSolver>()),
      m_dynamicsWorld(std::make_unique<btDiscreteDynamicsWorld>(m_dispatcher.get(), m_broadphase.get(), m_solver.get(),
                                                                m_collisionConfig.get()))
{
    m_dynamicsWorld->setGravity(btVector3(0, static_cast<float>(GravityY), 0));
    m_dynamicsWorld->getSolverInfo().m_numIterations = SolverIterations;
}

// デストラクタ：確保した全てのリソースを解放する
PhysicsWorld::~PhysicsWorld()
{
    // 登録されているオブジェクトに親の削除を通知
    for (auto* obj : m_registeredObjects)
    {
        obj->m_world = nullptr;
    }
}

// シミュレーションを進める
void PhysicsWorld::step(float deltaTime) { m_dynamicsWorld->stepSimulation(deltaTime, MaxSubSteps); }

RaycastResult PhysicsWorld::raycast(const s3d::Ray& ray, double maxDistance)
{
    const Vec3 origin = ray.getOrigin();
    const Vec3 direction = ray.getDirection();
    const btVector3 from = ToBtVector3(origin);
    const btVector3 to = ToBtVector3(origin + direction * maxDistance);

    btCollisionWorld::ClosestRayResultCallback callback(from, to);
    m_dynamicsWorld->rayTest(from, to, callback);

    if (callback.hasHit())
    {
        RaycastResult result;
        result.hasHit = true;
        result.hitPoint = ToSiv3DVec3(callback.m_hitPointWorld);
        result.hitNormal = ToSiv3DVec3(callback.m_hitNormalWorld);

        const btRigidBody* body = btRigidBody::upcast(callback.m_collisionObject);
        if (body && body->getUserPointer())
        {
            PhysicsBody* physicsBody = static_cast<PhysicsBody*>(body->getUserPointer());
            if (auto owner = physicsBody->getOwner())
            {
                result.hitObject = owner.get();
            }
        }
        return result;
    }

    return {}; // No hit
}

// 箱を作成する
std::unique_ptr<PhysicsBody> PhysicsWorld::createBox(const BoxDesc& desc)
{
    auto shape = std::make_unique<btBoxShape>(ToBtVector3(desc.size * 0.5));
    return std::make_unique<PhysicsBody>(this, std::move(shape), ShapeType::Box, desc.mass, desc.position);
}

// 球を作成する
std::unique_ptr<PhysicsBody> PhysicsWorld::createSphere(const SphereDesc& desc)
{
    auto shape = std::make_unique<btSphereShape>(desc.radius);
    return std::make_unique<PhysicsBody>(this, std::move(shape), ShapeType::Sphere, desc.mass, desc.position);
}

// 円柱を作成する
std::unique_ptr<PhysicsBody> PhysicsWorld::createCylinder(const CylinderDesc& desc)
{
    auto shape = std::make_unique<btCylinderShape>(btVector3{desc.radius, desc.height * 0.5, desc.radius});
    return std::make_unique<PhysicsBody>(this, std::move(shape), ShapeType::Cylinder, desc.mass, desc.position);
}

void PhysicsWorld::registerObject(PhysicsBody* obj)
{
    m_registeredObjects.insert(obj);
    m_dynamicsWorld->addRigidBody(obj->m_body.get());
}

void PhysicsWorld::unregisterObject(PhysicsBody* obj)
{
    if (m_registeredObjects.erase(obj) > 0)
    {
        if (m_dynamicsWorld)
        {
            m_dynamicsWorld->removeRigidBody(obj->m_body.get());
        }
    }
}
