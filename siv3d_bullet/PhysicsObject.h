#pragma once
#include <btBulletDynamicsCommon.h>
#include <Siv3D.hpp>


struct BoxDesc
{
	s3d::Vec3 size;
	s3d::Vec3 position;
	float mass;
};

struct SphereDesc {
	float radius;
	s3d::Vec3 position;
	float mass;
};
class PhysicsObject
{
	// Object は PhysicsWorldで管理する。
	friend class PhysicsWorld;

public:

	~PhysicsObject();

	void update();
	void draw();
	void setPosition(const s3d::Vec3& pos);
	void setRotation(const s3d::Vec3& rot);
	void setRestitution(float restitution);

	void setColor(const Color& color) { m_color = color; }

	s3d::Vec3 getPosition() const;
	s3d::Vec3 getRotation() const;

	void applyForce(const s3d::Vec3& force);
	void applyImpulse(const s3d::Vec3& impulse);

	btRigidBody* getRigidBody() const { return m_body; }

private:
	btRigidBody* m_body = nullptr;
	btCollisionShape* m_shape = nullptr;
	Color m_color = s3d::Palette::White;

	PhysicsObject(BoxDesc desc);
	PhysicsObject(SphereDesc desc);
	void init(btCollisionShape* shape, float mass, const s3d::Vec3& position);
};

