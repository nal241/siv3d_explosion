#pragma once
#include <Siv3D.hpp>
#include <vector>
#include <memory>
#include <algorithm>
#include "IComponent.h"

#include "CollisionGroups.h"
#include "PhysicsBody.h"
#include "Renderers.h"

class PhysicsWorld;

/// @brief ゲームオブジェクトの基底クラス
///
/// PhysicsBody（物理演算）とIRenderer（描画）を組み合わせて、
/// ゲーム内のオブジェクトを表現します。
///
/// 所有関係:
/// - GameObjectはPhysicsBodyとIRendererをunique_ptrで所有
/// - PhysicsBodyはGameObjectをweak_ptrで参照（循環参照を避けるため）
class GameObject : public std::enable_shared_from_this<GameObject>
{
public:
    using IDType = uint64;

    // 仮想デストラクタは、ポリモーフィズムを安全に使うために必須
    virtual ~GameObject() = default;

    // moveのみ許可（コピー禁止）
    GameObject(const GameObject&) = delete;
    GameObject& operator=(const GameObject&) = delete;
    GameObject(GameObject&&) = default;
    GameObject& operator=(GameObject&&) = default;

    // --- Component Management ---
    template <typename T, typename... Args>
    std::shared_ptr<T> addComponent(Args&&... args)
    {
        static_assert(std::is_base_of_v<IComponent, T>, "T must be a descendant of IComponent");
        auto newComponent = std::make_shared<T>(std::forward<Args>(args)...);
        newComponent->init(*this);
        m_components.push_back(newComponent);
        return newComponent;
    }

    template <typename T>
    std::shared_ptr<T> getComponent()
    {
        for (const auto& component : m_components)
        {
            if (auto casted = std::dynamic_pointer_cast<T>(component))
            {
                return casted;
            }
        }
        return nullptr;
    }

    // --- Core Methods ---
    virtual void update()
    {
        for (const auto& component : m_components)
        {
            component->update();
        }
    }
    virtual bool shouldBeRemoved() const { return m_shouldBeRemoved; }
    void destroy() { m_shouldBeRemoved = true; }

    void draw() const;
    void drawWireframe() const;

    // --- Getters / Setters ---
    IDType getID() const { return m_id; }

    void setPosition(const Vec3& pos);
    Vec3 getPosition() const;

    void setRotation(const Quaternion& rot);
    Quaternion getRotation() const;

    PhysicsBody* getPhysicsBody() { return m_physicsBody.get(); }

    // --- Static Factory Methods ---

    // パラメータ構造体
    struct BoxParams
    {
        Vec3 size;
        Vec3 position;
        float mass;                            // 0.0fで静的オブジェクト
        ColorF color = Linear::Palette::White; // デフォルトレンダラー用
        float restitution = 0.5f;
        float friction = 0.5f;
        CollisionGroup group = GROUP_DEFAULT;
        CollisionMask mask = MASK_ALL;
    };

    struct SphereParams
    {
        float radius;
        Vec3 position;
        float mass;
        ColorF color = Linear::Palette::White; // デフォルトレンダラー用
        float restitution = 0.5f;
        float friction = 0.5f;
        CollisionGroup group = GROUP_DEFAULT;
        CollisionMask mask = MASK_ALL;
    };

    struct CylinderParams
    {
        float radius;
        float height;
        Vec3 position;
        float mass;
        ColorF color = Linear::Palette::White; // デフォルトレンダラー用
        float restitution = 0.5f;
        float friction = 0.5f;
        CollisionGroup group = GROUP_DEFAULT;
        CollisionMask mask = MASK_ALL;
    };

    // 汎用Factory Methods（レンダラーはオプショナル）
    static std::shared_ptr<GameObject> CreateBox(PhysicsWorld& world, const BoxParams& params,
                                                 std::unique_ptr<IRenderer> renderer = nullptr);
    static std::shared_ptr<GameObject> CreateSphere(PhysicsWorld& world, const SphereParams& params,
                                                    std::unique_ptr<IRenderer> renderer = nullptr);
    static std::shared_ptr<GameObject> CreateCylinder(PhysicsWorld& world, const CylinderParams& params,
                                                      std::unique_ptr<IRenderer> renderer = nullptr);

    // コンストラクタ（Factoryからの使用を推奨）
    GameObject(std::unique_ptr<PhysicsBody> physicsBody, std::unique_ptr<IRenderer> renderer);

protected:
    // 子クラスからアクセスできるように protected にする
    IDType m_id;
    Vec3 m_position{0, 0, 0};
    Quaternion m_rotation = Quaternion::Identity();
    bool m_shouldBeRemoved = false;

private:
    static inline IDType s_nextID = 0;

    std::unique_ptr<PhysicsBody> m_physicsBody;
    std::unique_ptr<IRenderer> m_renderer;
    std::vector<std::shared_ptr<IComponent>> m_components;
};
