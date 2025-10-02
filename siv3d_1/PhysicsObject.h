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
public:
	PhysicsObject(BoxDesc desc);
	PhysicsObject(SphereDesc desc);
	~PhysicsObject();

	void update();
	void draw();
	void setPosition(const s3d::Vec3& pos);
	void setRotation(const s3d::Vec3& rot);

	s3d::Vec3 getPosition() const;
	s3d::Vec3 getRotation() const;

	void applyForce(const s3d::Vec3& force);

	btRigidBody* getRigidBody() const { return m_body; }

private:
	btRigidBody* m_body = nullptr;
	btCollisionShape* m_shape = nullptr;

	void init(btCollisionShape* shape, float mass, const s3d::Vec3& position);
};

