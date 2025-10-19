#pragma once
#include <Siv3D.hpp>

// 3D空間のパーティクル構造体
struct Particle3D
{
    s3d::Vec3 position;
    s3d::Vec3 velocity;
    s3d::ColorF color;
    double size;
    double life; // 残り寿命
    bool active;
};

// Forward declaration
class GameObject;

namespace ExplosionHelper
{
    void CreateExplosion(
        s3d::Array<Particle3D>& particles,
        const s3d::Array<std::shared_ptr<GameObject>>& gameObjects,
        const s3d::Vec3& center,
        double radius,
        const std::shared_ptr<GameObject>& bombObject = nullptr
    );
}