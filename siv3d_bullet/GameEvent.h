#pragma once
#include <Siv3D.hpp>

// Forward declaration
class GameObject;

/// @brief 爆発リクエストイベント
struct ExplosionRequest
{
    s3d::Vec3 position;
    double radius;
    std::weak_ptr<GameObject> source; // 爆発源
};

/// @brief 敵撃破イベント
struct EnemyDefeatedEvent
{
    int baseScore; // 基礎スコア
};

/// @brief すべてのゲームイベントの型
/// 新しいイベント型を追加する場合は、ここにvariantの型として追加する
using GameEvent = std::variant<ExplosionRequest, EnemyDefeatedEvent>;
