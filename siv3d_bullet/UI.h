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

class UI
{
public:
    UI();

    void update(double deltaTime);

    void draw() const;

    bool handleClick();

    void startReload(ItemType itemType);

    ItemType getSelectedItem() const { return m_selectedItem; }

    bool canUseSelectedItem() const;

    // コンボ表示用（表示のみ担当）
    void setComboInfo(int comboCount, double multiplier, double remainingTime);

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

    // UI描画の定数
    static constexpr double IconScale = 0.4;
    static constexpr double IconOffsetY = 30.0;
    static constexpr double NameFontSize = 18.0;
    static constexpr double NameOffsetY = 20.0;
    static constexpr double ReloadBarHPadding = 5.0;
    static constexpr double ReloadBarVOffsetY = 5.0;
    static constexpr double ReloadBarHeight = 5.0;

    // コンボ表示用のメンバ変数
    int m_displayComboCount = 0;
    double m_displayMultiplier = 1.0;
    double m_displayRemainingTime = 0.0;

    // コンボ表示用のフォント
    Font m_comboFont{40, Typeface::Bold};
    Font m_multiplierFont{24};
};
