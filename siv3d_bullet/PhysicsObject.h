#pragma once
#include <btBulletDynamicsCommon.h>
#include <Siv3D.hpp>

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

class PhysicsObject
{
public:
    ~PhysicsObject();

    // 削除禁止（moveのみ許可）
    PhysicsObject(const PhysicsObject&) = delete;
    PhysicsObject& operator=(const PhysicsObject&) = delete;
    PhysicsObject(PhysicsObject&&) = default;
    PhysicsObject& operator=(PhysicsObject&&) = default;

    PhysicsObject(PhysicsWorld* world, std::unique_ptr<btCollisionShape> shape, ShapeType type, float mass,
                  const s3d::Vec3& position);

    void update();
    void draw();
    void setPosition(const s3d::Vec3& pos);
    void setRotation(const s3d::Vec3& rot);
    void setRestitution(float restitution);
    void setFriction(float friction);
    void setDamping(float lin_damping, float ang_damping);

    void setColor(const Color& color) { m_color = color; }
    void setModel(const s3d::Model& model) { m_model = model; }

    s3d::Vec3 getPosition() const { return m_position; }
    s3d::Quaternion getRotation() const { return m_rotation; }

    void applyForce(const s3d::Vec3& force);
    void applyImpulse(const s3d::Vec3& impulse);

	// 爆弾ように追加
	float getMass() const;

    // btRigidBody *getRigidBody() const { return m_body; }

private:
    // Object は PhysicsWorldで管理する。
    friend class PhysicsWorld;

    std::unique_ptr<btCollisionShape> m_shape;
    std::unique_ptr<btRigidBody> m_body;
    std::unique_ptr<btMotionState> m_motionState;

    PhysicsWorld* m_world = nullptr;
    ShapeType m_shapeType;

    s3d::Vec3 m_position;
    s3d::Quaternion m_rotation;

    // 描画
    s3d::Optional<s3d::Model> m_model;
    Color m_color = s3d::Linear::Palette::White;
};
