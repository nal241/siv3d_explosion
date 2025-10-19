#include "Bomb.h"
#include "GameObject.h"
#include "ExplosionComponent.h"
#include "PhysicsWorld.h"

std::shared_ptr<GameObject> Bomb::Create(PhysicsWorld& world, const GameObject::SphereParams& params)
{
    // 1. まずは通常の球体ゲームオブジェクトとして生成
    auto bombObject = GameObject::CreateSphere(world, params);
    if (!bombObject)
    {
        return nullptr;
    }

    // 2. 爆発コンポーネントを追加
    bombObject->addComponent<ExplosionComponent>();

    // 3. 設定済みのオブジェクトを返す
    return bombObject;
}
