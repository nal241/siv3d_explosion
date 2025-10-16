#pragma once
#include <Siv3D.hpp>
#include <memory>

#include "Renderers.h"
#include "PhysicsBody.h"

class GameObject
{
public:
    using IDType = uint64;

    GameObject() : m_id(s_nextID++) {}

    // 仮想デストラクタは、ポリモーフィズムを安全に使うために必須
    virtual ~GameObject() = default;

    // 削除禁止
    GameObject(const GameObject&) = delete;
    GameObject& operator=(const GameObject&) = delete;
    GameObject(GameObject&&) = delete;
    GameObject& operator=(GameObject&&) = delete;

    void update();
    void draw() const;

    // --- Setters ---
    void setPhysicsBody(std::unique_ptr<PhysicsBody> physicsBody);
    void setRenderer(std::unique_ptr<IRenderer> renderer);

    // --- Getters / Setters ---
    IDType getID() const { return m_id; }

    void setPosition(const Vec3& pos) { m_position = pos; }
    const Vec3& getPosition() const { return m_position; }

    void setRotation(const Quaternion& rot) { m_rotation = rot; }
    const Quaternion& getRotation() const { return m_rotation; }

    PhysicsBody* getPhysicsBody() { return m_physicsBody.get(); }

protected:
    // 子クラスからアクセスできるように protected にする
    IDType m_id;
    Vec3 m_position{0, 0, 0};
    Quaternion m_rotation = Quaternion::Identity();

private:
    static inline IDType s_nextID = 0;

    std::unique_ptr<PhysicsBody> m_physicsBody;
    std::unique_ptr<IRenderer> m_renderer;
};
