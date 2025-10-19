#pragma once
#include <btBulletDynamicsCommon.h>
#include <Siv3D.hpp>

// Bullet → Siv3D
inline s3d::Vec3 ToSiv3DVec3(const btVector3& v) { return s3d::Vec3(v.x(), v.y(), v.z()); }
inline s3d::Quaternion ToSiv3DQuaternion(const btQuaternion& q) { return s3d::Quaternion(q.x(), q.y(), q.z(), q.w()); }

// Siv3D → Bullet
inline btVector3 ToBtVector3(const s3d::Vec3& v)
{
    return btVector3(static_cast<float>(v.x), static_cast<float>(v.y), static_cast<float>(v.z));
}
inline btQuaternion ToBtQuaternion(const s3d::Quaternion& q)
{
    return btQuaternion(static_cast<float>(q.getX()), static_cast<float>(q.getY()), static_cast<float>(q.getZ()),
                        static_cast<float>(q.getW()));
}
