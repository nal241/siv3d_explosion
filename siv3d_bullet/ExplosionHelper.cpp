#include "ExplosionHelper.h"
#include "GameObject.h"
#include "PhysicsBody.h"
#include "Enemy.h"

namespace
{
    // === パーティクル設定 ===
    constexpr int32 ParticleCount = 50;
    constexpr double MinParticleSpeed = 3.0;
    constexpr double MaxParticleSpeed = 8.0;
    constexpr double MinParticleSize = 0.2;
    constexpr double MaxParticleSize = 0.5;
    constexpr double MinParticleLife = 0.8;
    constexpr double MaxParticleLife = 1.5;
    constexpr double MinParticleHue = 0.0;
    constexpr double MaxParticleHue = 60.0;
    constexpr double MinParticleSaturation = 0.7;
    constexpr double MaxParticleSaturation = 1.0;

    // === 爆発の物理パラメータ ===
    constexpr double ExplosionBasePower = 10.0;
    constexpr double ExplosionMinDistance = 0.01;
}

void ExplosionHelper::CreateExplosion(
    s3d::Array<Particle3D>& particles,
    const s3d::Array<std::shared_ptr<GameObject>>& gameObjects,
    const s3d::Vec3& center,
    double radius,
    const std::shared_ptr<GameObject>& bombObject)
{
    s3d::Print << U"   Creating {} particles"_fmt(ParticleCount);

    // === パーティクル生成 ===
    for (int32 i = 0; i < ParticleCount; ++i)
    {
        const double theta = s3d::Random(0.0, s3d::Math::TwoPi);
        const double phi = s3d::Random(0.0, s3d::Math::Pi);
        const double speed = s3d::Random(MinParticleSpeed, MaxParticleSpeed);

        s3d::Vec3 direction{s3d::Math::Sin(phi) * s3d::Math::Cos(theta), s3d::Math::Sin(phi) * s3d::Math::Sin(theta), s3d::Math::Cos(phi)};

        Particle3D particle{
            .position = center,
            .velocity = direction * speed,
            .color = s3d::HSV{s3d::Random(MinParticleHue, MaxParticleHue),
                              s3d::Random(MinParticleSaturation, MaxParticleSaturation), 1.0},
            .size = s3d::Random(MinParticleSize, MaxParticleSize),
            .life = s3d::Random(MinParticleLife, MaxParticleLife),
            .active = true
        };

        particles << particle;
    }

    s3d::Print << U"   Applying force to objects...";

    // === 物理演算：オブジェクトに力を加える ===
    int32 hitCount = 0;
    for (auto& object : gameObjects)
    {
        if (object == bombObject)
            continue;

        auto body = object->getPhysicsBody();
        if (!body || body->isStatic())
            continue;

        s3d::Vec3 objectPos = object->getPosition();
        s3d::Vec3 direction = objectPos - center;
        double distance = direction.length();

        if (distance < radius && distance > ExplosionMinDistance)
        {
            s3d::Vec3 normalizedDirection = direction.normalized();
            double falloff = 1.0 - (distance / radius);
            double explosionForce = ExplosionBasePower * falloff;
            s3d::Vec3 force = normalizedDirection * explosionForce;

            body->applyImpulse(force);
            hitCount++;

            // エネミーにダメージを与える
            if (auto enemy = std::dynamic_pointer_cast<Enemy>(object))
            {
                int damage = static_cast<int>(falloff * 100);
                enemy->takeDamage(damage);
                s3d::Print << U"  → Hit Enemy: distance {:.2f}, damage {}"_fmt(distance, damage);
            }
            else
            {
                s3d::Print << U"  → Hit: distance {:.2f}, force {:.2f}"_fmt(distance, explosionForce);
            }
        }
    }
    s3d::Print << U"   Hit {} objects"_fmt(hitCount);
}