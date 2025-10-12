#pragma once
#include "SceneCommon.h"

#include "PhysicsWorld.h"
#include "Player.h"
class Explosion : public App::Scene
{
public:
    Explosion(const InitData& init);

    void update() override;

    void draw() const override;
};
