#include "GameObject.h"
#include "PhysicsWorld.h"

// --- Constructor ---

GameObject::GameObject(std::unique_ptr<PhysicsBody> physicsBody, std::unique_ptr<IRenderer> renderer)
    : m_id(s_nextID++), m_physicsBody(std::move(physicsBody)), m_renderer(std::move(renderer))
{
    // PhysicsBodyが指定されていれば、初期位置・回転を同期
    if (m_physicsBody)
    {
        // PhysicsBodyにポインタを設定
        m_physicsBody->setOwner(this);

        m_position = m_physicsBody->getPosition();
        m_rotation = m_physicsBody->getRotation();
    }
}

// --- Public Methods ---

void GameObject::draw() const
{
    if (m_renderer)
    {
        m_renderer->draw(getPosition(), getRotation());
    }
}

void GameObject::drawWireframe() const
{
    if (m_renderer)
    {
        m_renderer->drawWireframe(getPosition(), getRotation());
    }
}

// --- Getters / Setters ---

Vec3 GameObject::getPosition() const
{
    if (m_physicsBody)
    {
        return m_physicsBody->getPosition();
    }
    else
    {
        return m_position;
    }
}

Quaternion GameObject::getRotation() const
{
    if (m_physicsBody)
    {
        return m_physicsBody->getRotation();
    }
    else
    {
        return m_rotation;
    }
}

void GameObject::setPosition(const Vec3& pos)
{
    m_position = pos; // 基準位置を更新
    if (m_physicsBody)
    {
        m_physicsBody->setPosition(pos); // 物理ボディも更新
    }
}

void GameObject::setRotation(const Quaternion& rot)
{
    m_rotation = rot; // 基準回転を更新
    if (m_physicsBody)
    {
        m_physicsBody->setRotation(rot); // 物理ボディも更新
    }
}

// --- Static Factory Methods ---

std::unique_ptr<GameObject> GameObject::CreateBox(PhysicsWorld& world, const BoxParams& params,
                                                  std::unique_ptr<IRenderer> renderer)
{
    auto body = world.createBox(BoxDesc{params.size, params.position, params.mass});
    body->setRestitution(params.restitution);
    body->setFriction(params.friction);

    // レンダラーが指定されていなければデフォルト（PhysicsShapeRenderer）を使用
    if (!renderer)
    {
        renderer = std::make_unique<PhysicsShapeRenderer>(*body, params.color);
    }

    // GameObjectを生成
    return std::make_unique<GameObject>(std::move(body), std::move(renderer));
}

std::unique_ptr<GameObject> GameObject::CreateSphere(PhysicsWorld& world, const SphereParams& params,
                                                     std::unique_ptr<IRenderer> renderer)
{
    auto body = world.createSphere(SphereDesc{params.radius, params.position, params.mass});
    body->setRestitution(params.restitution);
    body->setFriction(params.friction);

    // レンダラーが指定されていなければデフォルト（PhysicsShapeRenderer）を使用
    if (!renderer)
    {
        renderer = std::make_unique<PhysicsShapeRenderer>(*body, params.color);
    }

    // GameObjectを生成
    return std::make_unique<GameObject>(std::move(body), std::move(renderer));
}

std::unique_ptr<GameObject> GameObject::CreateCylinder(PhysicsWorld& world, const CylinderParams& params,
                                                       std::unique_ptr<IRenderer> renderer)
{
    auto body = world.createCylinder(CylinderDesc{params.radius, params.height, params.position, params.mass});
    body->setRestitution(params.restitution);
    body->setFriction(params.friction);

    // レンダラーが指定されていなければデフォルト（PhysicsShapeRenderer）を使用
    if (!renderer)
    {
        renderer = std::make_unique<PhysicsShapeRenderer>(*body, params.color);
    }

    // GameObjectを生成
    return std::make_unique<GameObject>(std::move(body), std::move(renderer));
}