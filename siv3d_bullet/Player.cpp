#include "Player.h"
#include "GameObject.h"

namespace
{
    // Cube settings
    constexpr s3d::Vec3 CubeSize{1.0, 1.0, 1.0};
    constexpr float CubeMass = 0.3f;
    constexpr float CubeRestitution = 0.7f;

    // Sphere settings
    constexpr float SphereRadius = 0.5f;
    constexpr float SphereMass = 0.3f;
    constexpr float SphereRestitution = 0.7f;

    // Coin settings
    constexpr float CoinRadius = 0.5f;
    constexpr float CoinHeight = 0.2f;
    constexpr float CoinMass = 0.1f;
    constexpr float CoinRestitution = 0.0f;
    constexpr float CoinFriction = 0.1f;

    // common settings for launch
    constexpr double LaunchImpulse = 1.0;
} // namespace

Player::Player(DebugCamera3D* camera, Model& coinModel) : m_camera(camera), m_coinModel(coinModel) {}

void Player::handleInput(PhysicsWorld& world, HashTable<GameObject::IDType, std::unique_ptr<GameObject>>& objects)
{
    // スペースキーでキューブを発射
    if (m_inputs.shootBox.down())
    {
        launchObject(ObjectType::Box, world, objects);
    }

    // oキーでsphereを発射
    if (m_inputs.shootSphere.down())
    {
        launchObject(ObjectType::Sphere, world, objects);
    }

    if (m_inputs.shootCoin.down())
    {
        launchObject(ObjectType::Coin, world, objects);
    }
}

void Player::launchObject(ObjectType type, PhysicsWorld& world,
                          HashTable<GameObject::IDType, std::unique_ptr<GameObject>>& objects)
{ // カメラの位置と前方ベクトルを取得
    assert(m_camera != nullptr && "Camera pointer must not be null. Did you forget to call setCamera?");
    Vec3 camPos = m_camera->getEyePosition();
    Vec3 camForward = m_camera->getLookAtVector();

    // 初期位置（カメラの少し前）
    Vec3 initialPos = camPos + camForward * 20.0;

    // オブジェクト生成
    std::unique_ptr<PhysicsBody> newPhysicsBody;
    auto newGameObject = std::make_unique<GameObject>();

    switch (type)
    {
    case ObjectType::Box:
    {
        // キューブ生成
        newPhysicsBody = world.createBox(BoxDesc{CubeSize, initialPos, CubeMass});
        newPhysicsBody->setRestitution(CubeRestitution);
        newGameObject->setColor(s3d::Linear::Palette::Gainsboro);
        break;
    }
    case ObjectType::Sphere:
    {
        // 球生成
        newPhysicsBody = world.createSphere(SphereDesc{SphereRadius, initialPos, SphereMass});
        newPhysicsBody->setRestitution(SphereRestitution);
        newGameObject->setColor(s3d::Linear::Palette::Lightsteelblue);
        break;
    }
    case ObjectType::Coin:
    {
        // モデル付き円柱（コイン）を生成
        newPhysicsBody = world.createCylinder(CylinderDesc{CoinRadius, CoinHeight, initialPos, CoinMass});
        newPhysicsBody->setRestitution(CoinRestitution);
        newPhysicsBody->setFriction(CoinFriction);
        newPhysicsBody->setDamping(0.1f, 0.5f);
        newGameObject->setModel(m_coinModel);
        break;
    }
    }

    // 前方へインパルスを加える
    if (newPhysicsBody)
    {
        newPhysicsBody->applyImpulse(camForward * LaunchImpulse);
        newGameObject->setPhysicsBody(std::move(newPhysicsBody));
        objects.emplace(newGameObject->getID(), std::move(newGameObject));
        // 発射音を再生
        m_shootSound.playOneShot();
    }
}