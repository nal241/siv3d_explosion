#include "Renderers.h"
#include "BulletSiv3DUtils.h"

// --- ModelRenderer ---
ModelRenderer::ModelRenderer(const Model& model, const ColorF& color) : m_model(model), m_color(color) {}

void ModelRenderer::draw(const Vec3& position, const Quaternion& rotation) const { m_model.draw(position, rotation); }

void ModelRenderer::drawWireframe(const Vec3& position, const Quaternion& rotation) const
{
    const ScopedRenderStates3D states{RasterizerState::WireframeCullNone};
    m_model.draw(position, rotation);
}

// --- PhysicsShapeRenderer ---
PhysicsShapeRenderer::PhysicsShapeRenderer(const PhysicsBody& physicsBody, const ColorF& color)
    : m_physicsBody(physicsBody), m_color(color)
{
}

void PhysicsShapeRenderer::draw(const Vec3& position, const Quaternion& rotation) const
{
    const auto shapeType = m_physicsBody.getShapeType();
    const auto shape = m_physicsBody.getShape();

    switch (shapeType)
    {
    case ShapeType::Box:
    {
        auto boxShape = static_cast<btBoxShape*>(shape);
        s3d::Vec3 size = ToSiv3DVec3(boxShape->getHalfExtentsWithMargin()) * 2.0;
        s3d::OrientedBox obox(position, size, rotation);
        obox.draw(m_color);
        break;
    }
    case ShapeType::Sphere:
    {
        auto sphereShape = static_cast<btSphereShape*>(shape);
        double radius = sphereShape->getRadius();
        s3d::Sphere sphere(position, radius);
        sphere.draw(m_color);
        break;
    }
    case ShapeType::Cylinder:
    {
        auto cylinderShape = static_cast<btCylinderShape*>(shape);
        double radius = cylinderShape->getRadius();
        double height = cylinderShape->getHalfExtentsWithMargin().getY() * 2;
        s3d::Cylinder cylinder(position, radius, height, rotation);
        cylinder.draw(m_color);
        break;
    }
    }
}

void PhysicsShapeRenderer::drawWireframe(const Vec3& position, const Quaternion& rotation) const
{
    const ScopedRenderStates3D states{RasterizerState::WireframeCullNone};
    const auto shapeType = m_physicsBody.getShapeType();
    const auto shape = m_physicsBody.getShape();

    switch (shapeType)
    {
    case ShapeType::Box:
    {
        auto boxShape = static_cast<btBoxShape*>(shape);
        s3d::Vec3 size = ToSiv3DVec3(boxShape->getHalfExtentsWithMargin()) * 2.0;
        s3d::OrientedBox obox(position, size, rotation);
        obox.draw(Palette::Orange);
        break;
    }
    case ShapeType::Sphere:
    {
        auto sphereShape = static_cast<btSphereShape*>(shape);
        double radius = sphereShape->getRadius();
        s3d::Sphere sphere(position, radius);
        sphere.draw(Palette::Orange);
        break;
    }
    case ShapeType::Cylinder:
    {
        auto cylinderShape = static_cast<btCylinderShape*>(shape);
        double radius = cylinderShape->getRadius();
        double height = cylinderShape->getHalfExtentsWithMargin().getY() * 2;
        s3d::Cylinder cylinder(position, radius, height, rotation);
        cylinder.draw(Palette::Orange);
        break;
    }
    }
}

// --- TexturedShapeRenderer ---
TexturedShapeRenderer::TexturedShapeRenderer(const PhysicsBody& physicsBody, const Texture& texture, const Vec3& planeSize, const Vec2& uvTiling)
    : m_physicsBody(physicsBody), m_texture(texture), m_planeSize(planeSize), m_uvTiling(uvTiling)
{
    // Plane用のメッシュを作成（UVタイリングを指定）
    if (physicsBody.getShapeType() == ShapeType::Plane)
    {
        // 長方形のPlaneを作るため、より大きい方のサイズを基準にして正方形Meshを作る
        const double maxSize = Max(m_planeSize.x, m_planeSize.z);
        m_planeMesh = Mesh{MeshData::OneSidedPlane(maxSize, m_uvTiling)};
    }
}

void TexturedShapeRenderer::draw(const Vec3& position, const Quaternion& rotation) const
{
    const auto shapeType = m_physicsBody.getShapeType();
    const auto shape = m_physicsBody.getShape();

    switch (shapeType)
    {
    case ShapeType::Box:
    {
        auto boxShape = static_cast<btBoxShape*>(shape);
        s3d::Vec3 size = ToSiv3DVec3(boxShape->getHalfExtentsWithMargin()) * 2.0;
        s3d::OrientedBox obox(position, size, rotation);
        obox.draw(m_texture);
        break;
    }
    case ShapeType::Sphere:
    {
        auto sphereShape = static_cast<btSphereShape*>(shape);
        double radius = sphereShape->getRadius();
        s3d::Sphere sphere(position, radius);
        sphere.draw(m_texture);
        break;
    }
    case ShapeType::Cylinder:
    {
        auto cylinderShape = static_cast<btCylinderShape*>(shape);
        double radius = cylinderShape->getRadius();
        double height = cylinderShape->getHalfExtentsWithMargin().getY() * 2;
        s3d::Cylinder cylinder(position, radius, height, rotation);
        cylinder.draw(m_texture);
        break;
    }
    case ShapeType::Plane:
    {
        // Meshを使用してUVタイリングを適用したPlaneを描画
        const ScopedRenderStates3D sampler{SamplerState::RepeatLinear};

        // 長方形にするためのスケーリングを計算
        const double maxSize = Max(m_planeSize.x, m_planeSize.z);
        const Vec3 scale{m_planeSize.x / maxSize, 1.0, m_planeSize.z / maxSize};

        // Transformer3Dを使ってスケーリングを適用して描画
        const Transformer3D transformer{Mat4x4::Scale(scale)};
        m_planeMesh.draw(m_texture);
        break;
    }
    }
}

void TexturedShapeRenderer::drawWireframe(const Vec3& position, const Quaternion& rotation) const
{
    const ScopedRenderStates3D states{RasterizerState::WireframeCullNone};
    const auto shapeType = m_physicsBody.getShapeType();
    const auto shape = m_physicsBody.getShape();

    switch (shapeType)
    {
    case ShapeType::Box:
    {
        auto boxShape = static_cast<btBoxShape*>(shape);
        s3d::Vec3 size = ToSiv3DVec3(boxShape->getHalfExtentsWithMargin()) * 2.0;
        s3d::OrientedBox obox(position, size, rotation);
        obox.draw(Palette::Orange);
        break;
    }
    case ShapeType::Sphere:
    {
        auto sphereShape = static_cast<btSphereShape*>(shape);
        double radius = sphereShape->getRadius();
        s3d::Sphere sphere(position, radius);
        sphere.draw(Palette::Orange);
        break;
    }
    case ShapeType::Cylinder:
    {
        auto cylinderShape = static_cast<btCylinderShape*>(shape);
        double radius = cylinderShape->getRadius();
        double height = cylinderShape->getHalfExtentsWithMargin().getY() * 2;
        s3d::Cylinder cylinder(position, radius, height, rotation);
        cylinder.draw(Palette::Orange);
        break;
    }
    case ShapeType::Plane:
    {
        s3d::Plane plane(position, m_planeSize.x, m_planeSize.z);
        plane.draw(Palette::Orange);
        break;
    }
    }
}
