#include "Stage.h"
#include "PhysicsWorld.h"

Stage::Stage(std::unique_ptr<PhysicsBody> physicsBody, const StageParams& params)
    : GameObject(std::move(physicsBody), nullptr), // レンダラーはnullptr（自前で描画）
      m_roadWidth(params.roadWidth),
      m_grassWidth(params.grassWidth),
      m_depth(params.depth)
{
    // テクスチャの読み込み
    m_groundTexture = Texture{U"example/texture/ground.jpg", TextureDesc::Mipped};
    m_grassTexture = Texture{U"example/texture/grass.jpg", TextureDesc::Mipped};

    // 最大サイズ（奥行き）を基準にMeshを作成
    // --- 道路のMesh作成 ---
    // UVタイリング：実際のサイズ（幅x奥行き）に応じて設定
    const Vec2 roadUvTiling{m_roadWidth / 5.0, m_depth / 5.0};
    m_roadMesh = Mesh{MeshData::OneSidedPlane(m_depth, roadUvTiling)};

    // --- 草原のMesh作成 ---
    const Vec2 grassUvTiling{m_grassWidth / 5.0, m_depth / 5.0};
    m_grassLeftMesh = Mesh{MeshData::OneSidedPlane(m_depth, grassUvTiling)};
    m_grassRightMesh = Mesh{MeshData::OneSidedPlane(m_depth, grassUvTiling)};
}

std::shared_ptr<Stage> Stage::Create(PhysicsWorld& world, const StageParams& params)
{
    // 地面全体をカバーする大きなPlaneを作成
    const float totalWidth = params.grassWidth * 2 + params.roadWidth;

    auto planeBody = world.createPlane(
        PlaneDesc{
            .normal = Vec3{0, 1, 0},
            .distance = 0.0f,
            .position = params.position
        },
        GROUP_STATIC,
        MASK_ALL
    );

    planeBody->setRestitution(params.restitution);
    planeBody->setFriction(params.friction);

    // Stageオブジェクトを生成
    auto stage = std::shared_ptr<Stage>(new Stage(std::move(planeBody), params));
    stage->getPhysicsBody()->setOwner(stage->weak_from_this());

    return stage;
}

void Stage::draw() const
{
    const ScopedRenderStates3D sampler{SamplerState::RepeatLinear};

    // Meshは中心が原点、1000x1000の正方形が-500〜+500の範囲
    // スケール後: 例えば0.2倍すると200x1000になる（X:-100〜+100, Z:-500〜+500）

    // 配置:
    // 草原左: X範囲 [-250〜-50], 中心X=-150
    // 道路:   X範囲 [-25〜+25],  中心X=0
    // 草原右: X範囲 [+50〜+250], 中心X=+150

    // --- 草原（左側）の描画 ---
    {
        // 1000x1000のMeshを200x1000にスケール → [-100~+100, -500~+500]
        // それを X=-125 に移動 → [-225~-25, -500~+500]
        const Vec3 scale{m_grassWidth / m_depth, 1.0, 1.0};  // 0.2
        const double leftCenterX = -m_roadWidth / 2.0 - m_grassWidth / 2.0;  // -125
        const Vec3 leftPosition{leftCenterX, 0, m_depth / 2.0};
        // 先にスケール、後で移動
        const Mat4x4 transform = Mat4x4::Scale(scale).translated(leftPosition);
        const Transformer3D transformer{transform};
        m_grassLeftMesh.draw(m_grassTexture);
    }

    // --- 道路（中央）の描画 ---
    {
        // 1000x1000のMeshを50x1000にスケール → [-25~+25, -500~+500]
        const Vec3 scale{m_roadWidth / m_depth, 1.0, 1.0};  // 0.05
        const Vec3 roadPosition{0, 0, m_depth / 2.0};
        const Mat4x4 transform = Mat4x4::Scale(scale).translated(roadPosition);
        const Transformer3D transformer{transform};
        m_roadMesh.draw(m_groundTexture);
    }

    // --- 草原（右側）の描画 ---
    {
        // 1000x1000のMeshを200x1000にスケール → [-100~+100, -500~+500]
        // それを X=+125 に移動 → [+25~+225, -500~+500]
        const Vec3 scale{m_grassWidth / m_depth, 1.0, 1.0};  // 0.2
        const double rightCenterX = m_roadWidth / 2.0 + m_grassWidth / 2.0;  // +125
        const Vec3 rightPosition{rightCenterX, 0, m_depth / 2.0};
        const Mat4x4 transform = Mat4x4::Scale(scale).translated(rightPosition);
        const Transformer3D transformer{transform};
        m_grassRightMesh.draw(m_grassTexture);
    }
}

void Stage::drawWireframe() const
{
    // ワイヤーフレームでは地面全体の範囲を表示
    const ScopedRenderStates3D wireframe{RasterizerState::WireframeCullNone};
    const float totalWidth = m_grassWidth * 2 + m_roadWidth;
    Plane{getPosition(), totalWidth, m_depth}.draw(Palette::Orange);
}