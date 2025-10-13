#include "Renderer.h"
#include "PhysicsBody.h"
#include "BulletSiv3DUtils.h"

void Renderer::draw(const Vec3& position, const Quaternion& rotation, const Optional<Model>& model) const
{
    if (model)
    {
        model->draw(position, rotation);
    }
}

void Renderer::drawDebugShape(const Vec3& position, const Quaternion& rotation, PhysicsBody* physicsBody) const
{
    if (!physicsBody)
        return;

    const auto shapeType = physicsBody->getShapeType();
    const auto shape = physicsBody->getShape();

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
