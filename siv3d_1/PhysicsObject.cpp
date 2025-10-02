#include "PhysicsObject.h"

#include <Siv3D.hpp>

#include "BulletSiv3DUtils.h"

PhysicsObject::PhysicsObject(BoxDesc desc)
{
    init(new btBoxShape(ToBtVector3(desc.size * 0.5)), desc.mass, desc.position);
}

PhysicsObject::PhysicsObject(SphereDesc desc)
{
    init(new btSphereShape(desc.radius), desc.mass, desc.position);
}

PhysicsObject::~PhysicsObject()
{
    delete m_body;
    delete m_shape;
}

void PhysicsObject::draw()
{
    // Bulletから位置・回転を取得
    btTransform transform;
    m_body->getMotionState()->getWorldTransform(transform);
    s3d::Vec3 position = ToSiv3DVec3(transform.getOrigin());
    s3d::Quaternion rotation = ToSiv3DQuaternion(transform.getRotation());

    // 箱か球か判定
    if (auto boxShape = dynamic_cast<btBoxShape*>(m_shape))
    {
        s3d::Vec3 size = ToSiv3DVec3(boxShape->getHalfExtentsWithMargin()) * 2.0;
        s3d::OrientedBox obox(position, size, rotation);
        obox.draw(s3d::Palette::Skyblue);
    }
    else if (auto sphereShape = dynamic_cast<btSphereShape*>(m_shape))
    {
        double radius = sphereShape->getRadius();
        s3d::Sphere sphere(position, radius);
        sphere.draw(s3d::Palette::Orange);
    }
}

void PhysicsObject::init(btCollisionShape* shape, float mass, const s3d::Vec3& position)
{
	m_shape = shape;
	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(ToBtVector3(position));
	btDefaultMotionState* motionState = new btDefaultMotionState(transform);

	btVector3 localInertia(0, 0, 0);
	if (mass != 0.0f) {
		m_shape->calculateLocalInertia(mass, localInertia);
	}

	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, m_shape, localInertia);
	m_body = new btRigidBody(rbInfo);
}
