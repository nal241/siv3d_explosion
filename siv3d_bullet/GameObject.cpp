#include "GameObject.h"

void GameObject::update()
{
    if (m_physicsBody)
    {
        m_position = m_physicsBody->getPosition();
        m_rotation = m_physicsBody->getRotation();
    }
}

void GameObject::draw() const
{
    if (m_renderer)
    {
        m_renderer->draw(m_position, m_rotation);
    }
}

void GameObject::setPhysicsBody(std::unique_ptr<PhysicsBody> physicsBody)
{
    m_physicsBody = std::move(physicsBody);
}

void GameObject::setRenderer(std::unique_ptr<IRenderer> renderer)
{
    m_renderer = std::move(renderer);
}

