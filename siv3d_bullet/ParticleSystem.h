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

class ParticleSystem
{
public:
    ParticleSystem() = default;

    void update(double deltaTime);
    void draw() const;

    void add(const Particle3D& particle);

private:
    s3d::Array<Particle3D> m_particles;
    static constexpr s3d::Vec3 Gravity{0, -5.0, 0};
};
