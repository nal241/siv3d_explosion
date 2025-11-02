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
    const String text1_large = U"振";
    const String text1_normal = U"動!";
    const String text2 = U"Explosion!";
    const String text3 = U"モンスターズ!";

    const RectF r1_large = m_largeTitleFont(text1_large).region();
    const RectF r1_normal = m_titleFont(text1_normal).region();

    const double line1_width = r1_large.w + r1_normal.w;
    const double line1_y = 180;

    // Draw line 1
    m_largeTitleFont(text1_large).drawAt(Scene::Center().x - line1_width / 2 + r1_large.w / 2, line1_y, ColorF{1.0});
    m_titleFont(text1_normal).drawAt(Scene::Center().x - line1_width / 2 + r1_large.w + r1_normal.w / 2, line1_y, ColorF{1.0});

    // Draw line 2
    const double line2_y = line1_y + m_largeTitleFont.height() / 2;
    m_titleFont(text2).drawAt(Scene::Center().x, line2_y, ColorF{1.0});

    // Draw line 3
    const double line3_y = line2_y + m_titleFont.height();
    m_titleFont(text3).drawAt(Scene::Center().x, line3_y, ColorF{1.0});

    // メニュー表示
    m_menuFont(U"[Space] Start Game").drawAt(Scene::Center().x, 500, ColorF{0.8});
}
