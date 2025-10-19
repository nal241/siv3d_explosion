#pragma once

// 前方宣言
class GameObject;

class IComponent
{
public:
    IComponent() : m_owner(nullptr) {}
    virtual ~IComponent() = default;

    // コンポーネントがゲームオブジェクトに追加されたときに呼ばれる
    virtual void init(GameObject& owner) { m_owner = &owner; }

    // 毎フレームの更新
    virtual void update() = 0;

protected:
    GameObject* m_owner;
};
