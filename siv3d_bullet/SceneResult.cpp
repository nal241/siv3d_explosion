#include "SceneResult.h"

SceneResult::SceneResult(const InitData& init) : IScene{init} {}

void SceneResult::update()
{
    // Spaceキーでタイトルに戻る
    if (KeySpace.down())
    {
        changeScene(State::Title);
    }

    // Rキーでゲーム再開
    if (KeyR.down())
    {
        changeScene(State::Game);
    }
}

void SceneResult::draw() const
{
    Scene::SetBackground(ColorF{0.1, 0.2, 0.3});

    // リザルト表示
    m_titleFont(U"RESULT").drawAt(Scene::Center().x, 150, ColorF{1.0});

    // スコア表示
    m_scoreFont(U"Score: {}"_fmt(getData().score)).drawAt(Scene::Center().x, 300, ColorF{1.0, 0.8, 0.2});

    // メニュー表示
    m_menuFont(U"[Space] タイトルへ").drawAt(Scene::Center().x, 450, ColorF{0.8});

    m_menuFont(U"[R] もう一度プレイ").drawAt(Scene::Center().x, 500, ColorF{0.8});
}
