#include "ParticleSystem.h"

void ParticleSystem::update(double deltaTime)
{
    for (auto& p : m_particles)
    {
        if (not p.active)
        {
            continue;
        }

        p.velocity += Gravity * deltaTime;
        p.position += p.velocity * deltaTime;
        p.life -= deltaTime;

        if (p.life <= 0.0)
        {
            p.active = false;
        }
    }

    m_particles.remove_if([](const Particle3D& p) { return not p.active; });
}

void ParticleSystem::draw() const
{
    const ScopedRenderStates3D blend{BlendState::Additive};
    for (const auto& particle : m_particles)
    {
        if (!particle.active)
            continue;

        const double alpha = particle.life;
        const ColorF color = particle.color.withAlpha(alpha).removeSRGBCurve();
        Sphere{particle.position, particle.size}.draw(color);
    }
}

void ParticleSystem::add(const Particle3D& particle)
{
    m_particles.push_back(particle);
}
