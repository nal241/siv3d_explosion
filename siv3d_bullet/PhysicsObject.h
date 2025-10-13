#pragma once
#include <Siv3D.hpp>
#include <btBulletDynamicsCommon.h>

#include "GameObject.h"

class PhysicsWorld;

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

enum class ShapeType
{
    Box,
    Sphere,
    Cylinder,
};

class PhysicsObject : public GameObject
{
public:
    ~PhysicsObject();

    // move のみ許可
    PhysicsObject(const PhysicsObject&) = delete;
    PhysicsObject& operator=(const PhysicsObject&) = delete;
    PhysicsObject(PhysicsObject&&) = default;
    PhysicsObject& operator=(PhysicsObject&&) = default;

    PhysicsObject(PhysicsWorld* world, std::unique_ptr<btCollisionShape> shape, ShapeType type, float mass,
                  const s3d::Vec3& position);

    void update() override;
    void draw() const override;

    void setRestitution(float restitution);
    void setFriction(float friction);
    void setDamping(float lin_damping, float ang_damping);

    float getMass() const;

    void applyForce(const s3d::Vec3& force);
    void applyImpulse(const s3d::Vec3& impulse);

private:
    // Object は PhysicsWorldで管理する。
    friend class PhysicsWorld;

    std::unique_ptr<btCollisionShape> m_shape;
    std::unique_ptr<btRigidBody> m_body;
    std::unique_ptr<btMotionState> m_motionState;

    PhysicsWorld* m_world = nullptr;
    ShapeType m_shapeType;
};
