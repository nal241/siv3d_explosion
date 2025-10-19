#include "ExplosionComponent.h"

void ExplosionComponent::activate(double delay, double radius)
{
    // すでに起動中の場合は何もしない
    if (m_isPending)
    {
        return;
    }
    m_isPending = true;
    m_delay = delay;
    m_radius = radius;
    m_timer.restart();
}

void ExplosionComponent::update()
{
    if (!m_isPending)
    {
        return;
    }

    if (m_timer.sF() >= m_delay)
    {
        m_isPending = false;
    }
}

bool ExplosionComponent::isPending() const
{
    return m_isPending;
}

double ExplosionComponent::getRadius() const
{
    return m_radius;
}