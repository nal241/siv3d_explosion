#pragma once
#include "SceneCommon.h"

// タイトルシーン
class SceneTitle : public App::Scene
{
public:
    SceneTitle(const InitData& init);

    void update() override;

    void draw() const override;

private:
    Font m_largeTitleFont{70, Typeface::Bold};
    Font m_titleFont{60, Typeface::Bold};
    Font m_menuFont{30};
};
