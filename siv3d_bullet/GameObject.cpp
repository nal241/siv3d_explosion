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
        // モデルがあればモデルで描画
        if (m_model)
        {
            m_renderer->draw(m_position, m_rotation, m_model);
        }
        // なければ物理形状でデバッグ描画
        else if (m_physicsBody)
        {
            m_renderer->drawDebugShape(m_position, m_rotation, m_physicsBody.get());
        }
    }
}

void GameObject::setPhysicsBody(std::unique_ptr<PhysicsBody> physicsBody)
{
    m_physicsBody = std::move(physicsBody);
}

void GameObject::setRenderer(std::unique_ptr<Renderer> renderer)
{
    m_renderer = std::move(renderer);
}

