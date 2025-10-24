#pragma once
#include "GameObject.h"

/// @brief ステージ（地面）を表すGameObject
///
/// 物理判定は1枚の大きなPlane、描画は3つのMesh（草原左、道路、草原右）で構成される
class Stage : public GameObject
{
public:
    struct StageParams
    {
        float roadWidth = 50.0f;          // 道路の幅
        float grassWidth = 200.0f;        // 草原の幅（片側）
        float depth = 1000.0f;            // 奥行き
        Vec3 position = Vec3{0, 0, 0};    // 位置
        float restitution = 0.5f;         // 反発係数
        float friction = 0.8f;            // 摩擦係数
    };

    /// @brief Stageを作成するファクトリーメソッド
    static std::shared_ptr<Stage> Create(PhysicsWorld& world, const StageParams& params);

    /// @brief 描画（3つのMeshを描画）
    void draw() const override;

    /// @brief ワイヤーフレーム描画
    void drawWireframe() const override;

private:
    // コンストラクタ（Factoryから使用）
    Stage(std::unique_ptr<PhysicsBody> physicsBody, const StageParams& params);

    // 描画用メッシュ
    Mesh m_roadMesh;
    Mesh m_grassLeftMesh;
    Mesh m_grassRightMesh;

    // テクスチャ
    Texture m_groundTexture;
    Texture m_grassTexture;

    // サイズ情報（描画位置計算用）
    float m_roadWidth;
    float m_grassWidth;
    float m_depth;

    // UVタイリングの間隔（テクスチャ繰り返しの基準サイズ）
    static constexpr double UV_TILING_INTERVAL = 5.0;
};