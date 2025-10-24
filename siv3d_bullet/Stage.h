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
        double roadWidth = 10.0;       // 道路の幅
        double grassWidth = 30.0;      // 草原の幅（片側）
        double depth = 200.0;          // 奥行き
        Vec3 position = Vec3{0, 0, 0}; // 位置
        float restitution = 0.1f;      // 反発係数
        float friction = 0.8f;         // 摩擦係数
    };

    /// @brief Stageを作成するファクトリーメソッド
    static std::shared_ptr<Stage> Create(PhysicsWorld& world, const StageParams& params);

    /// @brief 描画（3つのMeshを描画）
    void draw() const override;

    /// @brief ワイヤーフレーム描画
    void drawWireframe() const override;

    /// @brief 木を追加
    void addTree(const Vec3& position, double scale, double rotationY);

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

    // モデル
    Model m_treeModel;
    Array<Mat4x4> m_treeTransforms;

    // サイズ情報（描画位置計算用）
    double m_roadWidth;
    double m_grassWidth;
    double m_depth;

    // UVタイリングの間隔（テクスチャ繰り返しの基準サイズ）
    static constexpr double UV_TILING_INTERVAL = 5.0;
};
