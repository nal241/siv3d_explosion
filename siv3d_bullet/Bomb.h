#pragma once
#include "GameObject.h"

// 前方宣言
class PhysicsWorld;

/// @brief 爆弾オブジェクトを生成するためのファクトリクラス
///
/// GameObject::CreateSphereを呼び出し、
/// 追加でExplosionComponentをアタッチして返します。
class Bomb
{
public:
    /// @brief 爆弾ゲームオブジェクトを生成します
    /// @param world 物理ワールド
    /// @param params 球のパラメータ
    /// @return 爆弾として設定されたGameObjectの共有ポインタ
    static std::shared_ptr<GameObject> Create(PhysicsWorld& world, const GameObject::SphereParams& params);
};
