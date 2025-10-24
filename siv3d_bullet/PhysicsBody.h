#pragma once
#include <Siv3D.hpp>
#include <btBulletDynamicsCommon.h>
#include "CollisionGroups.h"

// 前方宣言
class PhysicsWorld;
class GameObject;

// 物理形状の生成用Desc
struct BoxDesc
{
    s3d::Vec3 size;
    s3d::Vec3 position;
    float mass;
};

struct SphereDesc
{
    float radius;
    s3d::Vec3 position;
    float mass;
};

struct CylinderDesc
{
    float radius;
    float height;
    s3d::Vec3 position;
    float mass;
};

struct PlaneDesc
{
    s3d::Vec3 normal;   // 平面の法線ベクトル
    float distance;     // 原点からの距離
    s3d::Vec3 position; // 描画用の位置（物理演算では使われない）
};

struct ConvexHullDesc
{
    const s3d::Model* model; // モデルへのポインタ
    s3d::FilePath modelPath; // OBJファイルのパス（MeshData読み込み用）
    s3d::Vec3 position;
    float mass;
    s3d::Vec3 scale = s3d::Vec3{1.0, 1.0, 1.0}; // スケール（デフォルトは等倍）
};

enum class ShapeType
{
    Box,
    Sphere,
    Cylinder,
    Plane,
    ConvexHull,
};

class PhysicsBody
{
public:
    ~PhysicsBody();

    // move のみ許可
    PhysicsBody(const PhysicsBody&) = delete;
    PhysicsBody& operator=(const PhysicsBody&) = delete;
    PhysicsBody(PhysicsBody&&) = default;
    PhysicsBody& operator=(PhysicsBody&&) = default;

    PhysicsBody(PhysicsWorld* world, std::unique_ptr<btCollisionShape> shape, ShapeType type, const s3d::Vec3& position,
                float mass, CollisionGroup group, CollisionMask mask);

    void setRestitution(float restitution);
    void setFriction(float friction);
    void setDamping(float lin_damping, float ang_damping);
    void setOwner(std::weak_ptr<GameObject> owner);
    void setPosition(const s3d::Vec3& pos);
    void setRotation(const s3d::Quaternion& rot);

    float getMass() const;

    bool isStatic() const;

    void applyForce(const s3d::Vec3& force);
    void applyImpulse(const s3d::Vec3& impulse);

    // --- Getters ---
    btRigidBody* getBody() const { return m_body.get(); }
    s3d::Vec3 getPosition() const;
    s3d::Quaternion getRotation() const;
    ShapeType getShapeType() const { return m_shapeType; }
    btCollisionShape* getShape() const { return m_shape.get(); }
    std::weak_ptr<GameObject> getOwner() const;
    CollisionGroup getGroup() const { return m_group; }
    CollisionMask getMask() const { return m_mask; }
    PhysicsWorld* getWorld() const { return m_world; }

private:
    // PhysicsBody は PhysicsWorldで管理する。
    friend class PhysicsWorld;

    std::unique_ptr<btCollisionShape> m_shape;
    std::unique_ptr<btRigidBody> m_body;
    std::unique_ptr<btMotionState> m_motionState;

    PhysicsWorld* m_world = nullptr;
    std::weak_ptr<GameObject> m_owner;
    ShapeType m_shapeType;
    CollisionGroup m_group;
    CollisionMask m_mask;
};
