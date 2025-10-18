#include <Siv3D.hpp> // Siv3D v0.6.16

#include "SceneCommon.h"
#include "SceneGame.h"
#include "SceneTestExplosion.h"

void Main()
{
    App manager;
    // manager.add<Title>(State::Title);
    manager.add<SceneGame>(State::Game);
    // manager.add<Ranking>(State::Ranking);

    // 爆発用のシーン
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
