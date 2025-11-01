#pragma once
#include <Siv3D.hpp>

#include "PhysicsWorld.h"
#include "GameObject.h"

class Player
{
public:
    Player(BasicCamera3D* camera);

    void handleInput(PhysicsWorld& world, s3d::Array<std::shared_ptr<GameObject>>& objects);
    // void setCamera(BasicCamera3D* camera);  // 未実装：将来カメラ切り替えが必要になったら実装

private:
    struct Inputs
    {
        InputGroup shootBox{KeySpace};
        InputGroup shootSphere{KeyO};
    };

    enum class ObjectType
    {
        Box,
        Sphere,
    };

    void launchObject(ObjectType type, PhysicsWorld& world, s3d::Array<std::shared_ptr<GameObject>>& objects);

    BasicCamera3D* m_camera = nullptr;

    Inputs m_inputs;
};
