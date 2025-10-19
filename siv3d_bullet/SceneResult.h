#pragma once
#include "SceneCommon.h"

// リザルトシーン
class SceneResult : public App::Scene
{
public:
    SceneResult(const InitData& init);

    void update() override;

    void draw() const override;

private:
    Font m_titleFont{50, Typeface::Bold};
    Font m_scoreFont{60, Typeface::Bold};
    Font m_menuFont{30};
};
