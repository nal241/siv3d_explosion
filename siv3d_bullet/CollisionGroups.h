#pragma once
#include <cstdint>

// 物理オブジェクトのカテゴリ
enum CollisionGroup : int16_t
{
    GROUP_DEFAULT = 1,          // 通常の動的オブジェクト
    GROUP_STATIC = 1 << 1,      // 静的オブジェクト（地面、壁など）
    GROUP_ATTRACTABLE = 1 << 2, // 吸引力の影響を受けるオブジェクト
};

// どのカテゴリと衝突するかを指定するマスク
enum CollisionMask : int16_t
{
    MASK_ALL = -1,                                       // すべてのグループと衝突
    MASK_STATIC_ONLY = GROUP_STATIC,                     // 静的オブジェクトとのみ衝突
    MASK_ATTRACTABLE_ONLY = GROUP_ATTRACTABLE,           // 吸引可能なオブジェクトとのみ衝突
    MASK_NON_STATIC = GROUP_DEFAULT | GROUP_ATTRACTABLE, // 静的オブジェクト以外と衝突
    MASK_DEFAULT_ONLY = GROUP_DEFAULT,                   // デフォルトグループとのみ衝突
};
