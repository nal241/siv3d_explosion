#include "ParticleSystem.h"

void ParticleSystem::update(double deltaTime)
{
    for (auto& p : m_particles)
    {
        if (not p.active)
        {
            continue;
        }

        p.velocity += p.acceleration * deltaTime;
        p.position += p.velocity * deltaTime;
        p.life -= deltaTime;

        const double lifeRatio = 1.0 - (p.life / p.maxLife);
        // 開始色と終了色を補間
        p.color = p.startColor.lerp(p.endColor, lifeRatio);

        if (p.killZone && p.killZone->contains(p.position))
        {
            p.active = false;
        }

        if (p.life <= 0.0)
        {
            p.active = false;
        }
    }

    m_particles.remove_if([](const Particle3D& p) { return not p.active; });
}

void ParticleSystem::draw() const
{
    {
        const ScopedRenderStates3D blend{BlendState::Additive};
        for (const auto& particle : m_particles)
        {
            if (!particle.active || !particle.useAdditive)
                continue;

            const double lifeRatio = particle.life / particle.maxLife;
            const double alpha = s3d::Min(lifeRatio * 2.0, 1.0);
            const ColorF color = particle.color.withAlpha(alpha).removeSRGBCurve();
            Sphere{particle.position, particle.size}.draw(color);
        }
    }

    // Alpha blendingパーティクルを描画
    {
        const ScopedRenderStates3D blend{BlendState::NonPremultiplied};
        for (const auto& particle : m_particles)
        {
            if (!particle.active || particle.useAdditive)
                continue;

            const double lifeRatio = particle.life / particle.maxLife;
            const ColorF color = particle.color.withAlpha(particle.color.a * lifeRatio).removeSRGBCurve();
            Sphere{particle.position, particle.size}.draw(color);
        }
    }
}

void ParticleSystem::add(const Particle3D& particle)
{
    Particle3D p = particle;

    // maxLifeが未設定の場合、lifeを使用
    if (p.maxLife == 0.0)
    {
        p.maxLife = p.life;
    }

    // startColor/endColorが未設定（黒）の場合、colorを使用
    if (p.startColor == ColorF{0, 0, 0, 0} && p.endColor == ColorF{0, 0, 0, 0})
    {
        p.startColor = p.color;
        p.endColor = p.color;
    }

    m_particles.push_back(p);
}
