#include "Player.h"
#include "GameObject.h"
#include "Renderers.h"

namespace
{
    // 発射時の共通設定
    constexpr double LaunchImpulse = 1.0;

    // 発射オブジェクトのプリセット
    constexpr s3d::Vec3 DynamicBoxSize{1.0, 1.0, 1.0};
    constexpr float DynamicBoxMass = 0.3f;
    constexpr float DynamicBoxRestitution = 0.7f;

    constexpr float DynamicSphereRadius = 0.5f;
    constexpr float DynamicSphereMass = 0.3f;
    constexpr float DynamicSphereRestitution = 0.7f;

    constexpr float CoinRadius = 0.5f;
    constexpr float CoinHeight = 0.2f;
    constexpr float CoinMass = 0.1f;
    constexpr float CoinRestitution = 0.0f;
    constexpr float CoinFriction = 0.1f;
    constexpr float CoinLinearDamping = 0.1f;
    constexpr float CoinAngularDamping = 0.5f;
} // namespace

Player::Player(DebugCamera3D* camera, Model& coinModel) : m_camera(camera), m_coinModel(coinModel) {}

void Player::handleInput(PhysicsWorld& world, s3d::Array<std::shared_ptr<GameObject>>& objects)
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

void Player::launchObject(ObjectType type, PhysicsWorld& world, s3d::Array<std::shared_ptr<GameObject>>& objects)
{
    // カメラの位置と前方ベクトルを取得
    assert(m_camera != nullptr && "Camera pointer must not be null. Did you forget to call setCamera?");
    Vec3 camPos = m_camera->getEyePosition();
    Vec3 camForward = m_camera->getLookAtVector();

    // 初期位置（カメラの少し前）
    Vec3 initialPos = camPos + camForward * 20.0;

    // オブジェクト生成（factory methodsを使用）
    std::shared_ptr<GameObject> newGameObject;

    switch (type)
    {
    case ObjectType::Box:
        newGameObject = GameObject::CreateBox(world, GameObject::BoxParams{.size = DynamicBoxSize,
                                                                           .position = initialPos,
                                                                           .mass = DynamicBoxMass,
                                                                           .color = s3d::Linear::Palette::Gainsboro,
                                                                           .restitution = DynamicBoxRestitution});
        break;
    case ObjectType::Sphere:
        newGameObject =
            GameObject::CreateSphere(world, GameObject::SphereParams{.radius = DynamicSphereRadius,
                                                                     .position = initialPos,
                                                                     .mass = DynamicSphereMass,
                                                                     .color = s3d::Linear::Palette::Lightsteelblue,
                                                                     .restitution = DynamicSphereRestitution});
        break;
    case ObjectType::Coin:
        newGameObject = GameObject::CreateCylinder(world,
                                                   GameObject::CylinderParams{.radius = CoinRadius,
                                                                              .height = CoinHeight,
                                                                              .position = initialPos,
                                                                              .mass = CoinMass,
                                                                              .restitution = CoinRestitution,
                                                                              .friction = CoinFriction},
                                                   std::make_unique<ModelRenderer>(m_coinModel));
        // 追加設定: damping
        newGameObject->getPhysicsBody()->setDamping(CoinLinearDamping, CoinAngularDamping);
        break;
    }

    // 前方へインパルスを加える
    if (newGameObject)
    {
        newGameObject->getPhysicsBody()->applyImpulse(camForward * LaunchImpulse);
        objects.push_back(std::move(newGameObject));
        // 発射音を再生
        m_shootSound.playOneShot();
    }
}
