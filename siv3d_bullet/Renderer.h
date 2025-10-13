#pragma once
#include <Siv3D.hpp>

// 前方宣言
class PhysicsBody;

class Renderer
{
public:
    // モデルを使った描画
    void draw(const Vec3& position, const Quaternion& rotation, const Optional<Model>& model) const;

    // 物理形状を使ったデバッグ描画
    void drawDebugShape(const Vec3& position, const Quaternion& rotation, PhysicsBody* physicsBody) const;

    void setColor(ColorF color) { m_color = color; }

private:
    ColorF m_color = Linear::Palette::White;
};
