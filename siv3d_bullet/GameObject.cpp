#include "GameObject.h"
#include "PhysicsWorld.h"

// --- Constructor ---

GameObject::GameObject(std::unique_ptr<PhysicsBody> physicsBody, std::unique_ptr<IRenderer> renderer)
    : m_id(s_nextID++), m_physicsBody(std::move(physicsBody)), m_renderer(std::move(renderer))
{
    // 初期位置・回転をPhysicsBodyから同期
    if (m_physicsBody)
    {
        m_position = m_physicsBody->getPosition();
        m_rotation = m_physicsBody->getRotation();
    }
}

// --- Public Methods ---

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

// --- Private Methods ---

void GameObject::setPhysicsBody(std::unique_ptr<PhysicsBody> physicsBody)
{
    m_physicsBody = std::move(physicsBody);

    // PhysicsBody設定時に必ず位置・回転を同期（防御的プログラミング）
    if (m_physicsBody)
    {
        m_position = m_physicsBody->getPosition();
        m_rotation = m_physicsBody->getRotation();
    }
}

void GameObject::setRenderer(std::unique_ptr<IRenderer> renderer) { m_renderer = std::move(renderer); }

// --- Static Factory Methods の実装 ---

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

    // privateコンストラクタを使用してGameObjectを生成
    return std::unique_ptr<GameObject>(new GameObject(std::move(body), std::move(renderer)));
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

    // privateコンストラクタを使用してGameObjectを生成
    return std::unique_ptr<GameObject>(new GameObject(std::move(body), std::move(renderer)));
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

    // privateコンストラクタを使用してGameObjectを生成
    return std::unique_ptr<GameObject>(new GameObject(std::move(body), std::move(renderer)));
}
