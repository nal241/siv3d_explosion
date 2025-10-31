#include "PhysicsWorld.h"
#include "BulletSiv3DUtils.h"
#include "GameObject.h"
#include "PhysicsBody.h"

namespace
{
    constexpr double GravityY = -9.81;
    constexpr int SolverIterations = 10;
    constexpr int MaxSubSteps = 5;

    // 衝突コールバック
    bool contactAddedCallback(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0, int partId0, int index0,
                              const btCollisionObjectWrapper* colObj1, int partId1, int index1)
    {
        auto notifyCollision = [](const btCollisionObject* obj)
        {
            if (!(obj->getCollisionFlags() & btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK))
                return;
            const btRigidBody* body = btRigidBody::upcast(obj);
            if (body && body->getUserPointer())
            {
                PhysicsBody* physicsBody = static_cast<PhysicsBody*>(body->getUserPointer());
                physicsBody->onCollision();
            }
        };

        notifyCollision(colObj0->getCollisionObject());
        notifyCollision(colObj1->getCollisionObject());

        // falseを返すことで、Bulletの標準的な衝突応答をそのまま適用する
        return false;
    }
} // namespace

// コンストラクタ：ワールドのセットアップを行う
PhysicsWorld::PhysicsWorld()
    : m_collisionConfig(std::make_unique<btDefaultCollisionConfiguration>()),
      m_dispatcher(std::make_unique<btCollisionDispatcher>(m_collisionConfig.get())),
      m_broadphase(std::make_unique<btDbvtBroadphase>()),
      m_solver(std::make_unique<btSequentialImpulseConstraintSolver>()),
      m_dynamicsWorld(std::make_unique<btDiscreteDynamicsWorld>(m_dispatcher.get(), m_broadphase.get(), m_solver.get(),
                                                                m_collisionConfig.get())),
      m_debugDraw(std::make_unique<BulletDebugDraw>())
{
    m_dynamicsWorld->setGravity(btVector3(0, static_cast<float>(GravityY), 0));
    m_dynamicsWorld->getSolverInfo().m_numIterations = SolverIterations;

    // Bullet Physicsのグローバルな衝突コールバック関数を登録
    gContactAddedCallback = contactAddedCallback;
    // デバッグ描画を設定
    m_dynamicsWorld->setDebugDrawer(m_debugDraw.get());
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

// OBJファイルから頂点データを読み込む簡易パーサー
static s3d::Array<s3d::Vec3> LoadOBJVertices(const s3d::FilePath& path)
{
    s3d::Array<s3d::Vec3> vertices;
    s3d::TextReader reader(path);

    if (!reader)
    {
        throw std::runtime_error("Failed to open OBJ file");
    }

    s3d::String line;
    while (reader.readLine(line))
    {
        // 頂点行 "v x y z" をパース
        if (line.starts_with(U"v "))
        {
            // "v " 以降をスペースで分割
            const auto parts = line.substr(2).split(U' ');
            if (parts.size() >= 3)
            {
                const double x = s3d::ParseFloat<double>(parts[0]);
                const double y = s3d::ParseFloat<double>(parts[1]);
                const double z = s3d::ParseFloat<double>(parts[2]);
                vertices.push_back(s3d::Vec3{x, y, z});
            }
        }
    }

    return vertices;
}

// コンパウンドシェイプビルダーを作成
CompoundShapeBuilder PhysicsWorld::createCompoundShape() { return CompoundShapeBuilder(this); }

// Convex Hullを作成する
std::unique_ptr<PhysicsBody> PhysicsWorld::createConvexHull(const ConvexHullDesc& desc, CollisionGroup group,
                                                            CollisionMask mask)
{
    if (desc.modelPath.isEmpty())
    {
        throw std::invalid_argument("ConvexHullDesc::modelPath is empty");
    }

    // OBJファイルから頂点データを読み込む
    const s3d::Array<s3d::Vec3> objVertices = LoadOBJVertices(desc.modelPath);

    if (objVertices.isEmpty())
    {
        throw std::invalid_argument("Model has no vertices");
    }

    // 頂点データを収集してスケールを適用
    s3d::Array<btVector3> vertices;
    vertices.reserve(objVertices.size());

    for (const auto& vertex : objVertices)
    {
        // スケールを適用した頂点座標を追加
        s3d::Vec3 scaledPos = vertex * desc.scale;
        vertices.push_back(ToBtVector3(scaledPos));
    }

    // btConvexHullShapeを作成
    auto shape = std::make_unique<btConvexHullShape>(reinterpret_cast<const btScalar*>(vertices.data()),
                                                     static_cast<int>(vertices.size()), sizeof(btVector3));

    // 凸包の最適化（頂点数を減らして計算効率を向上）
    // NOTE: optimizeConvexHull()は形状を縮小する可能性があるため一旦コメントアウト
    // shape->optimizeConvexHull();

    return std::make_unique<PhysicsBody>(this, std::move(shape), ShapeType::ConvexHull, desc.position, desc.mass, group,
                                         mask);
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

// デバッグ描画を有効化
void PhysicsWorld::setDebugDrawEnabled(bool enabled) { m_debugDrawEnabled = enabled; }

// デバッグ描画モードを設定
void PhysicsWorld::setDebugDrawMode(int mode)
{
    if (m_debugDraw)
    {
        m_debugDraw->setDebugMode(mode);
    }
}

// デバッグ描画を実行
void PhysicsWorld::debugDraw() const
{
    if (!m_debugDrawEnabled || !m_debugDraw)
        return;

    // 前フレームの線をクリア
    m_debugDraw->clearLines();

    // Bulletにデバッグ描画を実行させる（m_debugDrawに線データが蓄積される）
    m_dynamicsWorld->debugDrawWorld();

    // 蓄積された線を実際に描画
    m_debugDraw->render();
}

CompoundShapeBuilder::CompoundShapeBuilder(PhysicsWorld* world) : m_world(world) {}

CompoundShapeBuilder& CompoundShapeBuilder::addSphere(s3d::Vec3 localPos, float radius, s3d::Quaternion localRot)
{
    auto shape = std::make_unique<btSphereShape>(radius);

    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(ToBtVector3(localPos));
    transform.setRotation(ToBtQuaternion(localRot));

    m_children.push_back({std::move(shape), transform});
    return *this;
}

CompoundShapeBuilder& CompoundShapeBuilder::addEllipsoid(s3d::Vec3 localPos, s3d::Vec3 radii, s3d::Quaternion localRot)
{
    // btMultiSphereShapeで半径1の球を作成し、スケーリングで楕円にする
    btVector3 position(0, 0, 0);
    btScalar radius = 1.0f;
    auto shape = std::make_unique<btMultiSphereShape>(&position, &radius, 1);

    // 非一様スケールを適用して楕円体にする
    shape->setLocalScaling(ToBtVector3(radii));

    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(ToBtVector3(localPos));
    transform.setRotation(ToBtQuaternion(localRot));

    m_children.push_back({std::move(shape), transform});
    return *this;
}

CompoundShapeBuilder& CompoundShapeBuilder::addBox(s3d::Vec3 localPos, s3d::Vec3 size, s3d::Quaternion localRot)
{
    auto shape = std::make_unique<btBoxShape>(ToBtVector3(size * 0.5));

    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(ToBtVector3(localPos));
    transform.setRotation(ToBtQuaternion(localRot));

    m_children.push_back({std::move(shape), transform});
    return *this;
}

CompoundShapeBuilder& CompoundShapeBuilder::addCone(s3d::Vec3 localPos, float radius, float height,
                                                    s3d::Quaternion localRot)
{
    // btConeShapeはY軸方向の円錐
    auto shape = std::make_unique<btConeShape>(radius, height);

    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(ToBtVector3(localPos));
    transform.setRotation(ToBtQuaternion(localRot));

    m_children.push_back({std::move(shape), transform});
    return *this;
}

CompoundShapeBuilder& CompoundShapeBuilder::addCylinder(s3d::Vec3 localPos, float radius, float height,
                                                        s3d::Quaternion localRot)
{
    auto shape = std::make_unique<btCylinderShape>(btVector3{radius, height * 0.5, radius});

    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(ToBtVector3(localPos));
    transform.setRotation(ToBtQuaternion(localRot));

    m_children.push_back({std::move(shape), transform});
    return *this;
}

std::unique_ptr<PhysicsBody> CompoundShapeBuilder::build(s3d::Vec3 position, float mass, CollisionGroup group,
                                                         CollisionMask mask)
{
    if (m_children.isEmpty())
    {
        throw std::runtime_error("CompoundShapeBuilder: No child shapes added");
    }

    // btCompoundShapeを作成
    auto compoundShape = std::make_unique<btCompoundShape>();

    // 子シェイプを追加（所有権はcompoundShapeに移譲）
    for (auto& child : m_children)
    {
        compoundShape->addChildShape(child.localTransform, child.shape.release());
    }

    // PhysicsBodyを作成して返す
    return std::make_unique<PhysicsBody>(m_world, std::move(compoundShape), ShapeType::Compound, position, mass, group,
                                         mask);
}
