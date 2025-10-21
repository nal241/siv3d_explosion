#pragma once
#include <Siv3D.hpp>
#include "ParticleSystem.h"

// Forward declaration
class GameObject;

namespace ExplosionHelper
{
    void CreateExplosion(ParticleSystem& particleSystem, const s3d::Array<std::shared_ptr<GameObject>>& gameObjects,
                         const s3d::Vec3& center, double radius,
                         const std::shared_ptr<GameObject>& bombObject = nullptr);
}
