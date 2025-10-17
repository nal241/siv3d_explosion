#pragma once
#include <Siv3D.hpp>
#include <memory>

#include "Renderers.h"
#include "PhysicsBody.h"

class PhysicsWorld;

class GameObject
{
public:
    using IDType = uint64;

    // 仮想デストラクタは、ポリモーフィズムを安全に使うために必須
    virtual ~GameObject() = default;

    // 削除禁止
    GameObject(const GameObject&) = delete;
    GameObject& operator=(const GameObject&) = delete;
    GameObject(GameObject&&) = delete;
    GameObject& operator=(GameObject&&) = delete;

    void update();
    void draw() const;

    // --- Getters / Setters ---
    IDType getID() const { return m_id; }

    void setPosition(const Vec3& pos) { m_position = pos; }
    const Vec3& getPosition() const { return m_position; }

    void setRotation(const Quaternion& rot) { m_rotation = rot; }
    const Quaternion& getRotation() const { return m_rotation; }

    PhysicsBody* getPhysicsBody() { return m_physicsBody.get(); }

    // --- Static Factory Methods ---

    // パラメータ構造体
    struct BoxParams
    {
        Vec3 size;
        Vec3 position;
        float mass; // 0.0fで静的オブジェクト
        ColorF color = Linear::Palette::White;  // デフォルトレンダラー用
        float restitution = 0.5f;
        float friction = 0.5f;
    };

    struct SphereParams
    {
        float radius;
        Vec3 position;
        float mass;
        ColorF color = Linear::Palette::White;  // デフォルトレンダラー用
        float restitution = 0.5f;
        float friction = 0.5f;
    };

    struct CylinderParams
    {
        float radius;
        float height;
        Vec3 position;
        float mass;
        ColorF color = Linear::Palette::White;  // デフォルトレンダラー用
        float restitution = 0.5f;
        float friction = 0.5f;
    };

    // 汎用Factory Methods（レンダラーはオプショナル）
    static std::unique_ptr<GameObject> CreateBox(PhysicsWorld& world, const BoxParams& params,
                                                 std::unique_ptr<IRenderer> renderer = nullptr);
    static std::unique_ptr<GameObject> CreateSphere(PhysicsWorld& world, const SphereParams& params,
                                                     std::unique_ptr<IRenderer> renderer = nullptr);
    static std::unique_ptr<GameObject> CreateCylinder(PhysicsWorld& world, const CylinderParams& params,
                                                       std::unique_ptr<IRenderer> renderer = nullptr);

protected:
    // 子クラスからアクセスできるように protected にする
    IDType m_id;
    Vec3 m_position{0, 0, 0};
    Quaternion m_rotation = Quaternion::Identity();

private:
    // privateコンストラクタ（Factoryからのみ使用）
    GameObject(std::unique_ptr<PhysicsBody> physicsBody, std::unique_ptr<IRenderer> renderer);

    // private setters（Factoryとコンストラクタからのみ使用）
    void setPhysicsBody(std::unique_ptr<PhysicsBody> physicsBody);
    void setRenderer(std::unique_ptr<IRenderer> renderer);

    static inline IDType s_nextID = 0;

    std::unique_ptr<PhysicsBody> m_physicsBody;
    std::unique_ptr<IRenderer> m_renderer;
};
