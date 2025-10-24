#include "PhysicsBody.h"

#include <btBulletDynamicsCommon.h>

#include "BulletSiv3DUtils.h"
#include "GameObject.h"
#include "PhysicsWorld.h"

PhysicsBody::PhysicsBody(PhysicsWorld* world, std::unique_ptr<btCollisionShape> shape, ShapeType type,
                         const Vec3& position, float mass, CollisionGroup group, CollisionMask mask)
    : m_world(world), m_shape(std::move(shape)), m_shapeType(type), m_group(group), m_mask(mask)
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

PhysicsBody::~PhysicsBody()
{
    if (m_world)
    {
        m_world->unregisterObject(this);
        m_world = nullptr;
    }
}

void PhysicsBody::setRestitution(float restitution) { m_body->setRestitution(restitution); }

void PhysicsBody::setFriction(float friction) { m_body->setFriction(friction); }
void PhysicsBody::setDamping(float lin_damping, float ang_damping) { m_body->setDamping(lin_damping, ang_damping); }

void PhysicsBody::applyForce(const Vec3& force)
{
    if (m_body)
    {
        m_body->activate(true);
        m_body->applyCentralForce(ToBtVector3(force));
    }
}

void PhysicsBody::applyImpulse(const Vec3& impulse)
{
    if (m_body)
    {
        m_body->activate(true);
        m_body->applyCentralImpulse(ToBtVector3(impulse));
    }
}

void PhysicsBody::setPosition(const s3d::Vec3& pos)
{
    btTransform transform = m_body->getWorldTransform();
    transform.setOrigin(ToBtVector3(pos));
    m_body->setWorldTransform(transform);
}

void PhysicsBody::setRotation(const s3d::Quaternion& rot)
{
    btTransform transform = m_body->getWorldTransform();
    transform.setRotation(ToBtQuaternion(rot));
    m_body->setWorldTransform(transform);
}

void PhysicsBody::setOwner(std::weak_ptr<GameObject> owner)
{
    m_owner = std::move(owner);
    m_body->setUserPointer(this);
}

void PhysicsBody::setCollisionCallback(std::function<void()> callback)
{
    m_collisionCallback = std::move(callback);
    if (m_collisionCallback)
    {
        m_body->setCollisionFlags(m_body->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
    }
}

// 質量を取得
float PhysicsBody::getMass() const
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

bool PhysicsBody::isStatic() const
{
    if (!m_body)
        return true;

    return m_body->isStaticObject();
}

s3d::Vec3 PhysicsBody::getPosition() const
{
    btTransform transform;
    m_body->getMotionState()->getWorldTransform(transform);
    return ToSiv3DVec3(transform.getOrigin());
}

s3d::Quaternion PhysicsBody::getRotation() const
{
    btTransform transform;
    m_body->getMotionState()->getWorldTransform(transform);
    return ToSiv3DQuaternion(transform.getRotation());
}

std::weak_ptr<GameObject> PhysicsBody::getOwner() const { return m_owner; }
