#include "Explosion.h"

Explosion::Explosion(const InitData& init) : IScene(init)
{

}

void Explosion::update()
{
    // Tキーでゲームのシーンへ移動
    if (KeyT.down())
    {
        changeScene(State::Game, 1.0s);
    }
}

void Explosion::draw() const
{

}
