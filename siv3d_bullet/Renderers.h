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
    PhysicsShapeRenderer(const PhysicsBody& physicsBody, const ColorF& color = Palette::White);
    void draw(const Vec3& position, const Quaternion& rotation) const override;
    void drawWireframe(const Vec3& position, const Quaternion& rotation) const override;

private:
    const PhysicsBody& m_physicsBody; // 描画対象の物理ボディへの参照（読み取り専用）
    ColorF m_color;
};

// --- テクスチャ付き物理形状の描画 ---
class TexturedShapeRenderer : public IRenderer
{
public:
    TexturedShapeRenderer(const PhysicsBody& physicsBody, const Texture& texture, const Vec3& planeSize = Vec3{100, 0, 100}, const Vec2& uvTiling = Vec2{1, 1});
    void draw(const Vec3& position, const Quaternion& rotation) const override;
    void drawWireframe(const Vec3& position, const Quaternion& rotation) const override;

private:
    const PhysicsBody& m_physicsBody;
    Texture m_texture;
    Vec3 m_planeSize; // Plane描画用のサイズ（Plane以外では無視される）
    Vec2 m_uvTiling;  // テクスチャのタイリング（繰り返し）数
    Mesh m_planeMesh; // Plane用のメッシュ（遅延初期化）
};
