#pragma once
#include <Siv3D.hpp>

#include "PhysicsWorld.h"
#include "GameObject.h"

class Player
{
public:
    Player(BasicCamera3D* camera, Model& coinModel);

    void handleInput(PhysicsWorld& world, s3d::Array<std::shared_ptr<GameObject>>& objects);
    // void setCamera(BasicCamera3D* camera);  // 未実装：将来カメラ切り替えが必要になったら実装

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

    void launchObject(ObjectType type, PhysicsWorld& world, s3d::Array<std::shared_ptr<GameObject>>& objects);

    // 効果音ラボから音源は取得
    Audio m_shootSound{U"example/sounds/shoot.mp3"};
    BasicCamera3D* m_camera = nullptr;
    Model& m_coinModel;

    Inputs m_inputs;
};
