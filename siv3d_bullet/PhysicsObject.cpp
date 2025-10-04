#include "PhysicsObject.h"

#include <Siv3D.hpp>

#include "BulletSiv3DUtils.h"
#include "PhysicsWorld.h"

PhysicsObject::PhysicsObject(PhysicsWorld* world, std::unique_ptr<btCollisionShape> shape, ShapeType type, float mass,
                             const s3d::Vec3& position)
    : m_world(world), m_shape(std::move(shape)), m_shapeType(type)
{
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(ToBtVector3(position));

    m_motionState = std::make_unique<btDefaultMotionState>(transform);

    btVector3 localInertia(0, 0, 0);
    if (mass != 0.0f)
    {
        m_shape->calculateLocalInertia(mass, localInertia);
    }

    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, m_motionState.get(), m_shape.get(), localInertia);

    m_body = std::make_unique<btRigidBody>(rbInfo);

    // Worldに登録
    if (m_world)
    {
        m_world->registerObject(this);
    }
}

PhysicsObject::~PhysicsObject()
{
    if (m_world)
    {
        m_world->unregisterObject(this);
        m_world = nullptr;
    }
}

void PhysicsObject::draw()
{
    // Bulletから位置・回転を取得
    btTransform transform;
    m_body->getMotionState()->getWorldTransform(transform);
    s3d::Vec3 position = ToSiv3DVec3(transform.getOrigin());
    s3d::Quaternion rotation = ToSiv3DQuaternion(transform.getRotation());

    switch (m_shapeType)
    {
    case ShapeType::Box:
    {
        auto boxShape = static_cast<btBoxShape*>(m_shape.get());
        s3d::Vec3 size = ToSiv3DVec3(boxShape->getHalfExtentsWithMargin()) * 2.0;
        s3d::OrientedBox obox(position, size, rotation);
        obox.draw(m_color);
        break;
    }
    case ShapeType::Sphere:
    {
        auto sphereShape = static_cast<btSphereShape*>(m_shape.get());
        double radius = sphereShape->getRadius();
        s3d::Sphere sphere(position, radius);
        sphere.draw(m_color);
        break;
    }
    }
}

void PhysicsObject::setRestitution(float restitution) { m_body->setRestitution(restitution); }

void PhysicsObject::applyImpulse(const s3d::Vec3& impulse) { m_body->applyCentralImpulse(ToBtVector3(impulse)); }
