#include "ItemSelectorUI.h"

namespace
{
    // リロード時間
    constexpr double BombReloadTime = 5.0;
    constexpr double DefaultReloadTime = 3.0;
}

ItemSelectorUI::ItemSelectorUI()
    : m_selectedItem(ItemType::Bomb),
      m_font(FontMethod::MSDF, 24, Typeface::Bold)
{
    m_itemInfos.push_back({U"Bomb 💣", Texture{U"💣"_emoji}, BombReloadTime});
    m_itemInfos.push_back({U"Gravity 🌀", Texture{U"🌀"_emoji}, DefaultReloadTime});
    m_itemInfos.push_back({U"Freeze ❄️", Texture{U"❄️"_emoji}, DefaultReloadTime});
    m_itemInfos.push_back({U"Wind 💨", Texture{U"💨"_emoji}, DefaultReloadTime});

    m_reloadTimers.resize(m_itemInfos.size(), 0.0);
}

void ItemSelectorUI::update(double deltaTime)
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

Rect ItemSelectorUI::getButtonRect(int32 index) const
{
    const int32 totalWidth = (ButtonWidth * 4) + (ButtonSpacing * 3);
    const int32 startX = (Scene::Width() - totalWidth) / 2;
    const int32 y = Scene::Height() - ButtonHeight - 20;
    const int32 x = startX + index * (ButtonWidth + ButtonSpacing);
    return Rect{x, y, ButtonWidth, ButtonHeight};
}

void ItemSelectorUI::draw() const
{
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
        info.emoji.scaled(0.4).drawAt(buttonRect.center().x, buttonRect.y + 30);

        // アイテム名
        m_font(info.name).drawAt(18, buttonRect.center().x, buttonRect.y + ButtonHeight - 20, Palette::White);

        // リロードバー
        if (!isReady)
        {
            const double progress = 1.0 - (reloadTimer / info.reloadTime);
            const RectF reloadBar{buttonRect.x + 5.0, buttonRect.y + 5.0, (ButtonWidth - 10.0) * progress, 5.0};
            reloadBar.draw(ColorF{0.9, 0.8, 0.3});
        }

        // マウスオーバー時のカーソル変更
        if (buttonRect.mouseOver())
        {
            Cursor::RequestStyle(CursorStyle::Hand);
        }
    }
}

bool ItemSelectorUI::handleClick()
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

void ItemSelectorUI::startReload(ItemType itemType)
{
    const int32 index = static_cast<int32>(itemType);
    m_reloadTimers[index] = m_itemInfos[index].reloadTime;
}

bool ItemSelectorUI::canUseSelectedItem() const
{
    const int32 index = static_cast<int32>(m_selectedItem);
    return m_reloadTimers[index] <= 0.0;
}
