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
        const Vec2 comboPos{Scene::Center().x, 100};
        const ColorF comboColor = HSV{30, 0.8, 1.0, alpha}; // オレンジ色

        const String comboText = U"{} COMBO"_fmt(m_displayComboCount);

        // 影を描画
        m_comboFont(comboText).drawAt(comboPos.movedBy(2, 2), ColorF{0, 0, 0, alpha * 0.5});
        // メインテキスト
        m_comboFont(comboText).drawAt(comboPos, comboColor);

        // 最新のコンボで得たスコア
        if (m_latestComboScore.baseScore > 0)
        {
            const String scoreText = U"{} pts"_fmt(m_latestComboScore.finalScore);
            const ColorF scoreColor = HSV{60, 0.8, 1.0, alpha}; // 黄色
            m_multiplierFont(scoreText).drawAt(comboPos.movedBy(0, 50), scoreColor);
        }
    }

    // スコアと残り時間の表示（画面上部）
    {
        // スコア表示（左上）
        const String scoreText = U"Score: {}"_fmt(m_displayScore);
        const Vec2 scorePos{20, 20};
        const ColorF scoreColor{1.0, 1.0, 1.0};

        // 影を描画
        m_gameInfoFont(scoreText).draw(scorePos.movedBy(2, 2), ColorF{0, 0, 0, 0.5});
        // メインテキスト
        m_gameInfoFont(scoreText).draw(scorePos, scoreColor);

        // 残り時間の表示（右上）
        const int seconds = static_cast<int>(m_displayRemainingGameTime);
        const String timeText = U"Time: {:>3}s"_fmt(seconds);
        const Vec2 timePos{Scene::Width() - 250, 20};

        // 時間が10秒以下の場合は赤色で警告
        const ColorF timeColor = (m_displayRemainingGameTime <= 10.0) ? ColorF{1.0, 0.2, 0.2} : ColorF{1.0, 1.0, 1.0};

        // 影を描画
        m_gameInfoFont(timeText).draw(timePos.movedBy(2, 2), ColorF{0, 0, 0, 0.5});
        // メインテキスト
        m_gameInfoFont(timeText).draw(timePos, timeColor);
    }

    // コンボ終了結果の表示
    if (m_comboResultTimer.isStarted() && m_comboResultTimer.sF() < m_comboResultDuration)
    {
        // 最後の0.5秒でフェードアウト
        double alpha = 1.0;
        const double remainingTime = m_comboResultDuration - m_comboResultTimer.sF();
        const double fadeOutDuration = 0.5;
        if (remainingTime < fadeOutDuration)
        {
            alpha = remainingTime / fadeOutDuration;
        }

        const Vec2 comboPos{Scene::Center().x, 100}; // m_displayComboCountと同じ位置
        const ColorF textColor = HSV{200, 0.8, 1.0, alpha}; // 水色

        const String resultText = U"{} COMBO!"_fmt(m_comboResultCount);
        const String scoreText = U"Total: {} pts"_fmt(m_comboResultScore);

        // 影
        m_comboFont(resultText).drawAt(comboPos.movedBy(2, 2), ColorF{0, 0, 0, alpha * 0.5});
        m_scoreFont(scoreText).drawAt(comboPos.movedBy(0, 50).movedBy(2, 2), ColorF{0, 0, 0, alpha * 0.5});

        // テキスト
        m_comboFont(resultText).drawAt(comboPos, textColor);
        m_scoreFont(scoreText).drawAt(comboPos.movedBy(0, 50), textColor);
    }

    // ゲーム終了通知の表示
    if (m_showGameOver)
    {
        const String gameOverText = U"TIME UP!";
        const Vec2 center = Scene::Center();

        // 背景の半透明黒
        Scene::Rect().draw(ColorF{0, 0, 0, 0.7});

        // 影を描画
        m_gameOverFont(gameOverText).drawAt(center.movedBy(3, 3), ColorF{0, 0, 0, 0.8});
        // メインテキスト
        m_gameOverFont(gameOverText).drawAt(center, ColorF{1.0, 0.2, 0.2});
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

void UI::setComboInfo(int comboCount, double multiplier, double remainingTime, int totalScore,
                      const ComboScoreInfo& latestScore)
{
    m_displayComboCount = comboCount;
    m_displayMultiplier = multiplier;
    m_displayRemainingTime = remainingTime;
    m_displayComboScore = totalScore;
    m_latestComboScore = latestScore;
}

void UI::setGameInfo(int score, double remainingTime)
{
    m_displayScore = score;
    m_displayRemainingGameTime = remainingTime;
}

void UI::showGameOver() { m_showGameOver = true; }

void UI::showComboResult(int score, int comboCount)
{
    // コンボ数が2以上の場合のみ表示
    if (comboCount > 1)
    {
        m_comboResultScore = score;
        m_comboResultCount = comboCount;
        m_comboResultTimer.restart();
    }
}
