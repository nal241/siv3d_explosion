#include "PhysicsWorld.h"
#include "BulletSiv3DUtils.h"
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

// 重力を取得
s3d::Vec3 PhysicsWorld::getGravity() const
{
    const btVector3 gravity = m_dynamicsWorld->getGravity();
    return ToSiv3DVec3(gravity);
}

RaycastResult PhysicsWorld::raycast(const s3d::Ray& ray, CollisionMask mask, double maxDistance)
{
    const Vec3 origin = ray.getOrigin();
    const Vec3 direction = ray.getDirection();
    const btVector3 from = ToBtVector3(origin);
    const btVector3 to = ToBtVector3(origin + direction * maxDistance);

    btCollisionWorld::ClosestRayResultCallback callback(from, to);
    callback.m_collisionFilterMask = mask;
    m_dynamicsWorld->rayTest(from, to, callback);

    if (callback.hasHit())
    {
        RaycastResult result;
        result.hasHit = true;
        result.hitPoint = ToSiv3DVec3(callback.m_hitPointWorld);
        result.hitNormal = ToSiv3DVec3(callback.m_hitNormalWorld);

        // ヒットしたオブジェクトのGameObjectを取得
        const btRigidBody* body = btRigidBody::upcast(callback.m_collisionObject);
        if (body && body->getUserPointer())
        {
            PhysicsBody* physicsBody = static_cast<PhysicsBody*>(body->getUserPointer());
            result.hitObject = physicsBody->getOwner();
        }
        return result;
    }

    return {}; // No hit
}

OverlapResult PhysicsWorld::overlapSphere(s3d::Vec3 center, double radius, CollisionMask mask)
{
    // Bulletの球体シェイプを作成
    btSphereShape sphereShape(static_cast<btScalar>(radius));

    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(ToBtVector3(center));

    btCollisionObject testObject;
    testObject.setCollisionShape(&sphereShape);
    testObject.setWorldTransform(transform);

    // コールバック定義
    struct OverlapCallback : public btCollisionWorld::ContactResultCallback
    {
        s3d::Array<std::weak_ptr<GameObject>> results;
        CollisionMask filterMask;

        OverlapCallback(CollisionMask mask) : filterMask(mask) {}

        btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0, int partId0, int index0,
                                 const btCollisionObjectWrapper* colObj1, int partId1, int index1) override
        {
            const btRigidBody* body = btRigidBody::upcast(colObj1->getCollisionObject());
            if (body && body->getUserPointer())
            {
                PhysicsBody* physicsBody = static_cast<PhysicsBody*>(body->getUserPointer());

                // マスクフィルタリング
                if ((physicsBody->getGroup() & filterMask) != 0)
                {
                    results.push_back(physicsBody->getOwner());
                }
            }
            return 0;
        }
    };

    OverlapCallback callback(mask);
    m_dynamicsWorld->contactTest(&testObject, callback);

    return OverlapResult{std::move(callback.results)};
}

// 箱を作成する
std::unique_ptr<PhysicsBody> PhysicsWorld::createBox(const BoxDesc& desc, CollisionGroup group, CollisionMask mask)
{
    auto shape = std::make_unique<btBoxShape>(ToBtVector3(desc.size * 0.5));
    return std::make_unique<PhysicsBody>(this, std::move(shape), ShapeType::Box, desc.position, desc.mass, group, mask);
}

// 球を作成する
std::unique_ptr<PhysicsBody> PhysicsWorld::createSphere(const SphereDesc& desc, CollisionGroup group,
                                                        CollisionMask mask)
{
    auto shape = std::make_unique<btSphereShape>(desc.radius);
    return std::make_unique<PhysicsBody>(this, std::move(shape), ShapeType::Sphere, desc.position, desc.mass, group,
                                         mask);
}

// 円柱を作成する
std::unique_ptr<PhysicsBody> PhysicsWorld::createCylinder(const CylinderDesc& desc, CollisionGroup group,
                                                          CollisionMask mask)
{
    auto shape = std::make_unique<btCylinderShape>(btVector3{desc.radius, desc.height * 0.5, desc.radius});
    return std::make_unique<PhysicsBody>(this, std::move(shape), ShapeType::Cylinder, desc.position, desc.mass, group,
                                         mask);
}

// 平面を作成する
std::unique_ptr<PhysicsBody> PhysicsWorld::createPlane(const PlaneDesc& desc, CollisionGroup group, CollisionMask mask)
{
    // btStaticPlaneShapeは無限平面を表す（質量は常に0で静的オブジェクト）
    auto shape = std::make_unique<btStaticPlaneShape>(ToBtVector3(desc.normal), desc.distance);
    return std::make_unique<PhysicsBody>(this, std::move(shape), ShapeType::Plane, desc.position, 0.0f, group, mask);
}

void PhysicsWorld::registerObject(PhysicsBody* obj)
{
    m_registeredObjects.insert(obj);
    m_dynamicsWorld->addRigidBody(obj->getBody(), obj->getGroup(), obj->getMask());
}

void PhysicsWorld::unregisterObject(PhysicsBody* obj)
{
    if (m_registeredObjects.erase(obj) > 0)
    {
        if (m_dynamicsWorld)
        {
            m_dynamicsWorld->removeRigidBody(obj->getBody());
        }
    }
}

// 放物線軌道で目標地点に到達するための初速度ベクトルを計算
//
// 前提条件:
//   - gravity.x == 0 && gravity.z == 0 (重力はY軸方向のみ)
//   - gravity.y < 0 (下向きの重力)
//   - launchAngleDeg は水平面からの角度（度数法）
//
// 戻り値:
//   指定された角度で target に到達するための初速度ベクトル
//   到達不可能な場合は std::nullopt
std::optional<Vec3> PhysicsWorld::CalculateLaunchVelocity(const Vec3& start, const Vec3& target,
                                                          const double launchAngleDeg, const Vec3& gravity)
{
    const Vec3 diff = target - start;
    const Vec3 diffXZ = {diff.x, 0.0, diff.z};
    const double distance = diffXZ.length();

    if (distance == 0.0)
        return std::nullopt;

    const double launchAngleRad = ToRadians(launchAngleDeg);
    const double cosAngle = Cos(launchAngleRad);
    const double tanAngle = Tan(launchAngleRad);

    // 放物運動の式から初速度の大きさを計算
    // v^2 = (g_y * d^2) / (2 * cos^2(θ) * (d * tan(θ) - h))
    // ここで d = 水平距離, h = 高さの差, g_y = gravity.y (負の値)
    const double v_pow2 =
        (-gravity.y * distance * distance) / (2.0 * cosAngle * cosAngle * (distance * tanAngle - diff.y));

    // v^2 が正でなければ到達不可能
    if (v_pow2 <= 0.0)
        return std::nullopt;

    const double v = Sqrt(v_pow2);

    return diffXZ.normalized() * v * cosAngle + Vec3{0, v * Sin(launchAngleRad), 0};
}
