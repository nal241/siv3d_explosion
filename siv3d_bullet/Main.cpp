#include <Siv3D.hpp> // Siv3D v0.6.16

#include "SceneCommon.h"
#include "SceneTitle.h"
#include "SceneGame.h"
#include "SceneResult.h"
#include "SceneTestExplosion.h"

void Main()
{
    App manager;
    manager.add<SceneTitle>(State::Title);
    manager.add<SceneGame>(State::Game);
    manager.add<SceneResult>(State::Result);
    manager.add<SceneTestExplosion>(State::Explosion);

    // システムループ
    while (System::Update())
    {
        if (not manager.update())
        {
            break;
        }
    }
}
