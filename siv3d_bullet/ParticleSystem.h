#pragma once
#include <Siv3D.hpp>

// 3D空間のパーティクル構造体
struct Particle3D
{
    s3d::Vec3 position;
    s3d::Vec3 velocity;
    s3d::Vec3 acceleration;
    s3d::ColorF color; // startColor/endColorが未設定時の色
    s3d::ColorF startColor = ColorF{0, 0, 0, 0}; // 開始色
    s3d::ColorF endColor = ColorF{0, 0, 0, 0};   // 終了色
    double size;
    double life;        // 残り寿命
    double maxLife = 0.0; // 最大寿命
    bool active;
    bool useAdditive = true; // true: Additive, false: Alpha blending
    s3d::Optional<s3d::Sphere> killZone;
};

class ParticleSystem
{
public:
    ParticleSystem() = default;

    void update(double deltaTime);
    void draw() const;

    void add(const Particle3D& particle);

private:
    s3d::Array<Particle3D> m_particles;
};
