#pragma once
#include <Siv3D.hpp>

#include "PhysicsWorld.h"
#include "GameObject.h"

class Player
{
public:
    Player(DebugCamera3D* camera, Model& coinModel);

    // void handleInput(PhysicsWorld& world, HashTable<GameObject::IDType, std::unique_ptr<GameObject>>& objects);
    void handleInput(PhysicsWorld& world, HashTable<GameObject::IDType, std::unique_ptr<GameObject>>& objects);
    void setCamera(DebugCamera3D* camera);

private:
    struct Inputs
    {
        InputGroup shootBox{KeySpace};
        InputGroup shootSphere{KeyO};
        InputGroup shootCoin{KeyC};
    };

    enum class ObjectType
    {
        Box,
        Sphere,
        Coin,
    };

    void launchObject(ObjectType type, PhysicsWorld& world,
                      HashTable<GameObject::IDType, std::unique_ptr<GameObject>>& objects);

    // 効果音ラボから音源は取得
    Audio m_shootSound{U"example/sounds/shoot.mp3"};
    DebugCamera3D* m_camera = nullptr;
    Model& m_coinModel;

    Inputs m_inputs;
};
