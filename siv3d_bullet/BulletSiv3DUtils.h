#pragma once
#include <btBulletDynamicsCommon.h>
#include <Siv3D.hpp>

// Bullet → Siv3D
inline s3d::Vec3 ToSiv3DVec3(const btVector3& v) { return s3d::Vec3(v.x(), v.y(), v.z()); }
inline s3d::Quaternion ToSiv3DQuaternion(const btQuaternion& q) { return s3d::Quaternion(q.x(), q.y(), q.z(), q.w()); }
inline s3d::ColorF ToSiv3DColor(const btVector3& v) { return s3d::ColorF(v.x(), v.y(), v.z()); }

// Siv3D → Bullet
inline btVector3 ToBtVector3(const s3d::Vec3& v)
{
    return btVector3(static_cast<float>(v.x), static_cast<float>(v.y), static_cast<float>(v.z));
}
inline btQuaternion ToBtQuaternion(const s3d::Quaternion& q)
{
    return btQuaternion(static_cast<float>(q.getX()), static_cast<float>(q.getY()), static_cast<float>(q.getZ()),
                        static_cast<float>(q.getW()));
}

/// @brief Bullet Physics のデバッグ描画をSiv3Dで実装するクラス
/// 衝突形状、AABB、コンタクトポイントなどを可視化
class BulletDebugDraw : public btIDebugDraw
{
public:
    BulletDebugDraw() : m_debugMode(DBG_DrawWireframe) {}

    // btIDebugDraw インターフェースの実装
    void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override
    {
        m_lines.push_back({ToSiv3DVec3(from), ToSiv3DVec3(to), ToSiv3DColor(color)});
    }

    void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar /*distance*/,
                          int /*lifeTime*/, const btVector3& color) override
    {
        // コンタクトポイントを描画（点と法線）
        s3d::Vec3 point = ToSiv3DVec3(PointOnB);
        s3d::Vec3 normal = ToSiv3DVec3(normalOnB);
        m_lines.push_back({point, point + normal * 0.5, ToSiv3DColor(color)});
    }

    void reportErrorWarning(const char* warningString) override
    {
        s3d::Logger << U"Bullet Warning: " << s3d::Unicode::FromUTF8(warningString);
    }

    void draw3dText(const btVector3& /*location*/, const char* /*textString*/) override
    {
        // 3Dテキスト描画は省略（必要に応じて実装）
    }

    void setDebugMode(int debugMode) override { m_debugMode = debugMode; }

    int getDebugMode() const override { return m_debugMode; }

    /// @brief デバッグ線を実際に描画
    void render() const
    {
        for (const auto& line : m_lines)
        {
            s3d::Line3D(line.from, line.to).draw(line.color);
        }
    }

    /// @brief フレーム開始時に呼び出し、前フレームの描画データをクリア
    void clearLines() { m_lines.clear(); }

private:
    int m_debugMode;

    // 描画する線のバッファ
    struct DebugLine
    {
        s3d::Vec3 from;
        s3d::Vec3 to;
        s3d::ColorF color;
    };
    s3d::Array<DebugLine> m_lines;
};
