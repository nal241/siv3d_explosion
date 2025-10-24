#include "SceneTitle.h"

SceneTitle::SceneTitle(const InitData& init) : IScene{init}
{
    // スコアをリセット
    getData().score = 0;
}

void SceneTitle::update()
{
    // Spaceキーでゲーム開始
    if (KeySpace.down())
    {
        changeScene(State::Game);
    }
}

void SceneTitle::draw() const
{
    Scene::SetBackground(ColorF{0.2, 0.3, 0.5});

    // タイトル表示
    m_titleFont(U"タイトル").drawAt(Scene::Center().x, 200, ColorF{1.0});

    // メニュー表示
    m_menuFont(U"[Space] ゲーム開始").drawAt(Scene::Center().x, 400, ColorF{0.8});
}
