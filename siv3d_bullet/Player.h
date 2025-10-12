#pragma once
#include <Siv3D.hpp>

#include "PhysicsWorld.h"

class Player
{
public:
    Player(DebugCamera3D* camera, Model& coinModel);

    enum class PlayerAction
    {
        ShootBox,
        ShootSphere,
        ShootCoin,
    };

    void executeAction(PlayerAction action);

    void handleInput(PhysicsWorld& world, Array<std::unique_ptr<PhysicsObject>>& objects);
    void setCamera(DebugCamera3D* camera);

private:
    // 効果音ラボから音源は取得
    Audio m_shootSound{U"example/sounds/shoot.mp3"};
    DebugCamera3D* m_camera = nullptr;
    Model& m_coinModel;
};
