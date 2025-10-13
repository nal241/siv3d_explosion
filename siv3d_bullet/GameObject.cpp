#include "GameObject.h"

void GameObject::draw() const
{
    if (m_model)
    {
        m_model->draw(m_position, m_rotation);
    }
}
