#pragma once
#include <Siv3D.hpp>

// シーンのステート
enum class State
{
    Title,
    Game,
    Result,
};

// 共有するデータ
struct GameData
{
    // ゲームのスコア
    int32 score = 0;
};

using App = SceneManager<State, GameData>;