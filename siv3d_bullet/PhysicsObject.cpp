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

    update();
}

PhysicsObject::~PhysicsObject()
{
    if (m_world)
    {
        m_world->unregisterObject(this);
        m_world = nullptr;
    }
}

// Bulletから位置・回転を取得し、メンバー変数にキャッシュする
void PhysicsObject::update()
{
    btTransform transform;
    m_body->getMotionState()->getWorldTransform(transform);
    m_position = ToSiv3DVec3(transform.getOrigin());
    m_rotation = ToSiv3DQuaternion(transform.getRotation());
}

void PhysicsObject::draw()
{
    if (m_model)
    {
        m_model->draw(m_position, m_rotation);
    }
    else
    {
        switch (m_shapeType)
        {
        case ShapeType::Box:
        {
            auto boxShape = static_cast<btBoxShape*>(m_shape.get());
            s3d::Vec3 size = ToSiv3DVec3(boxShape->getHalfExtentsWithMargin()) * 2.0;
            s3d::OrientedBox obox(m_position, size, m_rotation);
            obox.draw(m_color);
            break;
        }
        case ShapeType::Sphere:
        {
            auto sphereShape = static_cast<btSphereShape*>(m_shape.get());
            double radius = sphereShape->getRadius();
            s3d::Sphere sphere(m_position, radius);
            sphere.draw(m_color);
            break;
        }
        case ShapeType::Cylinder:
        {
            auto cylinderShape = static_cast<btCylinderShape*>(m_shape.get());
            double radius = cylinderShape->getRadius();
            double height = cylinderShape->getHalfExtentsWithMargin().getY() * 2;
            s3d::Cylinder cylinder(m_position, radius, height, m_rotation);
            cylinder.draw(m_color);
            break;
        }
        }
    }
}

void PhysicsObject::setRestitution(float restitution) { m_body->setRestitution(restitution); }

void PhysicsObject::setFriction(float friction) { m_body->setFriction(friction); };
void PhysicsObject::setDamping(float lin_damping, float ang_damping) { m_body->setDamping(lin_damping, ang_damping); };

void PhysicsObject::applyImpulse(const s3d::Vec3& impulse) { m_body->applyCentralImpulse(ToBtVector3(impulse)); }

// ★ 質量を取得
float PhysicsObject::getMass() const
{
    if (!m_body)
        return 0.0f;

    // 質量の逆数から質量を計算
    float invMass = m_body->getInvMass();

    // invMass が 0 なら、質量は無限大（静止オブジェクト）
    if (invMass == 0.0f)
        return 0.0f;

    return 1.0f / invMass;
}
