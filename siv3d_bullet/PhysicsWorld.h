#pragma once

#include <btBulletDynamicsCommon.h>

#include "CollisionGroups.h"
#include "PhysicsBody.h"
#include "BulletSiv3DUtils.h"

// 前方宣言
class GameObject;
class CompoundShapeBuilder;

struct RaycastResult
{
    bool hasHit = false;
    // NOTE: hitObjectはweak_ptrで保持。使用時にlock()して有効性を確認すること。
    std::weak_ptr<GameObject> hitObject;
    s3d::Vec3 hitPoint;
    s3d::Vec3 hitNormal;
};

struct OverlapResult
{
    // NOTE: hitObjectsはweak_ptrで保持。使用時にlock()して有効性を確認すること。
    s3d::Array<std::weak_ptr<GameObject>> hitObjects;
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

    /// @brief 球体範囲内のGameObjectを取得
    /// @param center 中心位置
    /// @param radius 半径
    /// @param mask 衝突マスク
    /// @return 範囲内のGameObject（weak_ptr配列）
    OverlapResult overlapSphere(s3d::Vec3 center, double radius, CollisionMask mask = MASK_ALL);

    // 重力取得
    s3d::Vec3 getGravity() const;

    // オブジェクト追加（unique_ptrで返す）
    std::unique_ptr<PhysicsBody> createBox(const BoxDesc& desc, CollisionGroup group, CollisionMask mask);
    std::unique_ptr<PhysicsBody> createSphere(const SphereDesc& desc, CollisionGroup group, CollisionMask mask);
    std::unique_ptr<PhysicsBody> createCylinder(const CylinderDesc& desc, CollisionGroup group, CollisionMask mask);
    std::unique_ptr<PhysicsBody> createPlane(const PlaneDesc& desc, CollisionGroup group, CollisionMask mask);
    std::unique_ptr<PhysicsBody> createConvexHull(const ConvexHullDesc& desc, CollisionGroup group, CollisionMask mask);

    CompoundShapeBuilder createCompoundShape();

    // --- static utilities ---
    static std::optional<Vec3> CalculateLaunchVelocity(const Vec3& start, const Vec3& target, double launchAngleDeg,
                                                       const Vec3& gravity);

    // --- デバッグ描画 ---
    /// @brief デバッグ描画を有効化
    void setDebugDrawEnabled(bool enabled);

    /// @brief デバッグ描画モードを設定（btIDebugDraw::DebugDrawModesの組み合わせ）
    void setDebugDrawMode(int mode);

    /// @brief デバッグ描画を実行（Graphics3D::SetCameraTransform()で設定されたカメラを使用）
    void debugDraw() const;

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

    // デバッグ描画（constメソッドから変更するためmutable）
    mutable std::unique_ptr<BulletDebugDraw> m_debugDraw;
    mutable bool m_debugDrawEnabled = false;

    // PhysicsObjectからの通知メソッド
    void registerObject(PhysicsBody* obj);
    void unregisterObject(PhysicsBody* obj);
};

/// @brief コンパウンドシェイプを構築するビルダークラス
class CompoundShapeBuilder
{
public:
    CompoundShapeBuilder(PhysicsWorld* world);

    /// @brief 球体を追加
    CompoundShapeBuilder& addSphere(s3d::Vec3 localPos, float radius,
                                    s3d::Quaternion localRot = s3d::Quaternion::Identity());

    /// @brief 楕円体を追加（btMultiSphereShape + スケーリング）
    /// @param localPos ローカル位置
    /// @param radii 各軸の半径 (x, y, z)
    /// @param localRot ローカル回転
    CompoundShapeBuilder& addEllipsoid(s3d::Vec3 localPos, s3d::Vec3 radii,
                                       s3d::Quaternion localRot = s3d::Quaternion::Identity());

    /// @brief 箱を追加
    CompoundShapeBuilder& addBox(s3d::Vec3 localPos, s3d::Vec3 size,
                                 s3d::Quaternion localRot = s3d::Quaternion::Identity());

    /// @brief 円錐を追加（Y軸方向）
    CompoundShapeBuilder& addCone(s3d::Vec3 localPos, float radius, float height,
                                  s3d::Quaternion localRot = s3d::Quaternion::Identity());

    /// @brief 円柱を追加（Y軸方向）
    CompoundShapeBuilder& addCylinder(s3d::Vec3 localPos, float radius, float height,
                                      s3d::Quaternion localRot = s3d::Quaternion::Identity());

    /// @brief コンパウンドシェイプをビルドしてPhysicsBodyを生成
    std::unique_ptr<PhysicsBody> build(s3d::Vec3 position, float mass, CollisionGroup group, CollisionMask mask);

private:
    struct ChildShapeData
    {
        std::unique_ptr<btCollisionShape> shape;
        btTransform localTransform;
    };

    PhysicsWorld* m_world;
    s3d::Array<ChildShapeData> m_children;
};
