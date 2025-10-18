#pragma once
#include <Siv3D.hpp>
#include "PhysicsBody.h"

// --- インターフェース ---
class IRenderer
{
public:
    virtual ~IRenderer() = default;
    virtual void draw(const Vec3& position, const Quaternion& rotation) const = 0;
    virtual void drawWireframe(const Vec3& position, const Quaternion& rotation) const = 0;
};

// --- モデル描画 ---
class ModelRenderer : public IRenderer
{
public:
    ModelRenderer(const Model& model, const ColorF& color = Palette::White);
    void draw(const Vec3& position, const Quaternion& rotation) const override;
    void drawWireframe(const Vec3& position, const Quaternion& rotation) const override;

private:
    Model m_model;
    ColorF m_color;
};

// --- 物理形状の描画 ---
class PhysicsShapeRenderer : public IRenderer
{
public:
    PhysicsShapeRenderer(PhysicsBody& physicsBody, const ColorF& color = Palette::White);
    void draw(const Vec3& position, const Quaternion& rotation) const override;
    void drawWireframe(const Vec3& position, const Quaternion& rotation) const override;

private:
    PhysicsBody& m_physicsBody; // 描画対象の物理ボディへの参照
    ColorF m_color;
};
