#include "UI.h"

namespace
{
    // リロード時間
    constexpr double BombReloadTime = 5.0;
    constexpr double DefaultReloadTime = 3.0;
    constexpr double FreezeReloadTime = 8.0;
} // namespace

UI::UI() : m_selectedItem(ItemType::Bomb), m_font(FontMethod::MSDF, 24, Typeface::Bold)

{
    m_itemInfos.push_back({U"Bomb", Texture{U"💣"_emoji}, BombReloadTime});
    m_itemInfos.push_back({U"Gravity", Texture{U"🌀"_emoji}, DefaultReloadTime});
    m_itemInfos.push_back({U"Freeze", Texture{U"❄️"_emoji}, FreezeReloadTime});
    m_itemInfos.push_back({U"Wind", Texture{U"💨"_emoji}, DefaultReloadTime});

    m_reloadTimers.resize(m_itemInfos.size(), 0.0);
}

void UI::update(double deltaTime)
{
    // リロードタイマーを更新
    for (auto& timer : m_reloadTimers)
    {
        if (timer > 0.0)
        {
            timer -= deltaTime;
            if (timer < 0.0)
            {
                timer = 0.0;
            }
        }
    }
}

Rect UI::getButtonRect(int32 index) const
{
    const int32 itemCount = static_cast<int32>(m_itemInfos.size());
    const int32 totalWidth = (ButtonWidth * itemCount) + (ButtonSpacing * (itemCount - 1));
    const int32 startX = (Scene::Width() - totalWidth) / 2;
    const int32 y = Scene::Height() - ButtonHeight - 20;
    const int32 x = startX + index * (ButtonWidth + ButtonSpacing);
    return Rect{x, y, ButtonWidth, ButtonHeight};
}

void UI::draw() const
{
    // アイテムボタン描画
    for (int32 i = 0; i < static_cast<int32>(m_itemInfos.size()); ++i)
    {
        const Rect buttonRect = getButtonRect(i);
        const ItemInfo& info = m_itemInfos[i];
        const bool isSelected = (static_cast<int32>(m_selectedItem) == i);
        const double reloadTimer = m_reloadTimers[i];
        const bool isReady = (reloadTimer <= 0.0);

        // 背景色
        if (isReady)
        {
            buttonRect.draw(ColorF{0.3, 0.5, 0.9, 0.8});
        }
        else
        {
            buttonRect.draw(ColorF{0.2, 0.2, 0.3, 0.8});
        }

        // 枠線
        if (isSelected)
        {
            buttonRect.drawFrame(3, 0, ColorF{1.0, 0.8, 0.2});
        }
        else
        {
            buttonRect.drawFrame(2, 0, ColorF{0.5, 0.7, 1.0});
        }

        // 絵文字
        info.emoji.scaled(UI::IconScale).drawAt(buttonRect.center().x, buttonRect.y + UI::IconOffsetY);

        // アイテム名
        m_font(info.name).drawAt(UI::NameFontSize, buttonRect.center().x,
                                 buttonRect.y + UI::ButtonHeight - UI::NameOffsetY, Palette::White);

        // リロードバー
        if (!isReady)
        {
            const double progress = 1.0 - (reloadTimer / info.reloadTime);
            const RectF reloadBar{buttonRect.x + UI::ReloadBarHPadding, buttonRect.y + UI::ReloadBarVOffsetY,
                                  (UI::ButtonWidth - UI::ReloadBarHPadding * 2) * progress, UI::ReloadBarHeight};
            reloadBar.draw(ColorF{0.9, 0.8, 0.3});
        }

        // マウスオーバー時のカーソル変更
        if (buttonRect.mouseOver())
        {
            Cursor::RequestStyle(CursorStyle::Hand);
        }
    }

    // コンボ表示（2コンボ以上の時のみ）
    if (m_displayComboCount > 1)
    {
        const double alpha = Math::Min(1.0, m_displayRemainingTime / 0.5); // 最後の0.5秒でフェードアウト

        // コンボ数を大きく表示
        const Vec2 comboPos{Scene::Center().x, 120};
        const ColorF comboColor = HSV{30, 0.8, 1.0, alpha}; // オレンジ色

        const String comboText = U"COMBO × {}"_fmt(m_displayComboCount);

        // 影を描画
        m_comboFont(comboText).drawAt(comboPos.movedBy(2, 2), ColorF{0, 0, 0, alpha * 0.5});
        // メインテキスト
        m_comboFont(comboText).drawAt(comboPos, comboColor);

        // 倍率表示
        const String multiplierText = U"× {:.1f}"_fmt(m_displayMultiplier);
        m_multiplierFont(multiplierText).drawAt(comboPos.movedBy(0, 50), ColorF{1.0, 1.0, 0.5, alpha});

        // コンボ期間中の総スコア表示
        const String scoreText = U"+{} pts"_fmt(m_displayComboScore);
        const ColorF scoreColor = HSV{120, 0.6, 1.0, alpha}; // 緑色
        m_scoreFont(scoreText).drawAt(comboPos.movedBy(0, 90), scoreColor);
    }
}

bool UI::handleClick()
{
    for (int32 i = 0; i < static_cast<int32>(m_itemInfos.size()); ++i)
    {
        const Rect buttonRect = getButtonRect(i);

        if (buttonRect.leftClicked())
        {
            m_selectedItem = static_cast<ItemType>(i);
            return true;
        }
    }

    return false;
}

void UI::startReload(ItemType itemType)
{
    const int32 index = static_cast<int32>(itemType);

    // 境界チェック
    if (index < 0 || static_cast<size_t>(index) >= m_reloadTimers.size())
    {
        return;
    }

    m_reloadTimers[index] = m_itemInfos[index].reloadTime;
}

bool UI::canUseSelectedItem() const
{
    const int32 index = static_cast<int32>(m_selectedItem);

    // 境界チェック
    if (index < 0 || static_cast<size_t>(index) >= m_reloadTimers.size())
    {
        return false;
    }

    return m_reloadTimers[index] <= 0.0;
}

void UI::setComboInfo(int comboCount, double multiplier, double remainingTime, int comboScore)
{
    m_displayComboCount = comboCount;
    m_displayMultiplier = multiplier;
    m_displayRemainingTime = remainingTime;
    m_displayComboScore = comboScore;
}
