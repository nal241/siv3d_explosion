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

    // Meshを作成（全て1000x1000の正方形、描画時にスケーリングして使用）
    const Vec2 roadUvTiling{m_roadWidth / UV_TILING_INTERVAL, m_depth / UV_TILING_INTERVAL};
    m_roadMesh = Mesh{MeshData::OneSidedPlane(m_depth, roadUvTiling)};

    const Vec2 grassUvTiling{m_grassWidth / UV_TILING_INTERVAL, m_depth / UV_TILING_INTERVAL};
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

    // 草原（左側）の描画
    {
        const Vec3 scale{m_grassWidth / m_depth, 1.0, 1.0};
        const double leftCenterX = -m_roadWidth / 2.0 - m_grassWidth / 2.0;
        const Vec3 leftPosition{leftCenterX, 0, m_depth / 2.0};
        const Mat4x4 transform = Mat4x4::Scale(scale).translated(leftPosition);
        const Transformer3D transformer{transform};
        m_grassLeftMesh.draw(m_grassTexture);
    }

    // 道路（中央）の描画
    {
        const Vec3 scale{m_roadWidth / m_depth, 1.0, 1.0};
        const Vec3 roadPosition{0, 0, m_depth / 2.0};
        const Mat4x4 transform = Mat4x4::Scale(scale).translated(roadPosition);
        const Transformer3D transformer{transform};
        m_roadMesh.draw(m_groundTexture);
    }

    // 草原（右側）の描画
    {
        const Vec3 scale{m_grassWidth / m_depth, 1.0, 1.0};
        const double rightCenterX = m_roadWidth / 2.0 + m_grassWidth / 2.0;
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