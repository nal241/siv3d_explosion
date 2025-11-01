#include "Renderers.h"
#include "BulletSiv3DUtils.h"

// --- ModelRenderer ---
ModelRenderer::ModelRenderer(const Model& model, const ColorF& color, double scale)
    : m_model(model), m_color(color), m_scale(scale)
{
}

void ModelRenderer::draw(const Vec3& position, const Quaternion& rotation) const
{
    // スケール、回転、位置を組み合わせた変換行列を作成
    const Mat4x4 mat = Mat4x4::Scale(m_scale) * Mat4x4::Rotate(rotation) * Mat4x4::Translate(position);
    m_model.draw(mat);
}

void ModelRenderer::drawWireframe(const Vec3& position, const Quaternion& rotation) const
{
    const ScopedRenderStates3D states{RasterizerState::WireframeCullNone};
    // スケール、回転、位置を組み合わせた変換行列を作成
    const Mat4x4 mat = Mat4x4::Scale(m_scale) * Mat4x4::Rotate(rotation) * Mat4x4::Translate(position);
    m_model.draw(mat);
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
    case ShapeType::ConvexHull:
        // ConvexHullの描画は複雑なため、BulletDebugDrawを使用してください
        break;
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
    case ShapeType::ConvexHull:
        // ConvexHullのワイヤーフレーム描画は複雑なため、BulletDebugDrawを使用してください
        break;
    }
}
