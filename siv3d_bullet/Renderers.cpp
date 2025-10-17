#include "Renderers.h"
#include "BulletSiv3DUtils.h"

// --- ModelRenderer ---
ModelRenderer::ModelRenderer(const Model& model, const ColorF& color) : m_model(model), m_color(color) {}

void ModelRenderer::draw(const Vec3& position, const Quaternion& rotation) const { m_model.draw(position, rotation); }

// --- PhysicsShapeRenderer ---
PhysicsShapeRenderer::PhysicsShapeRenderer(PhysicsBody& physicsBody, const ColorF& color)
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
