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

// 将来の拡張用（例）
// struct DamageEvent
// {
//     int damage;
//     s3d::Vec3 position;
// };
//
// struct ScoreEvent
// {
//     int points;
// };

/// @brief すべてのゲームイベントの型
/// 新しいイベント型を追加する場合は、ここにvariantの型として追加する
using GameEvent = std::variant<ExplosionRequest>;
// 将来の拡張例: using GameEvent = std::variant<ExplosionRequest, DamageEvent, ScoreEvent>;
