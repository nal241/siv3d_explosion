#pragma once
#include <Siv3D.hpp>

// アイテムの種類
enum class ItemType
{
    Bomb,
    Gravity,
    Freeze,
    Wind,
};

// アイテム情報
struct ItemInfo
{
    String name;
    Texture emoji;
    double reloadTime;
};

class ItemSelectorUI
{
public:
    ItemSelectorUI();

    void update(double deltaTime);

    void draw() const;

    bool handleClick();

    void startReload(ItemType itemType);

    ItemType getSelectedItem() const { return m_selectedItem; }

    bool canUseSelectedItem() const;

private:
    Rect getButtonRect(int32 index) const;

    // アイテム情報のテーブル
    Array<ItemInfo> m_itemInfos;

    // 現在選択されているアイテム
    ItemType m_selectedItem;

    // 各アイテムのリロード残り時間
    Array<double> m_reloadTimers;

    // フォント
    Font m_font;

    // ボタンサイズ
    static constexpr int32 ButtonWidth = 150;
    static constexpr int32 ButtonHeight = 80;
    static constexpr int32 ButtonSpacing = 20;
};
