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

// コンボ得点情報(1フレーム分)
struct ComboScoreInfo
{
    int baseScore;       // 基礎スコア
    double multiplier;   // 倍率
    int finalScore;      // 最終スコア(基礎スコア × 倍率)
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
    void setComboInfo(int comboCount, double multiplier, double remainingTime, int totalScore, const ComboScoreInfo& latestScore);

    // スコアと残り時間の表示用
    void setGameInfo(int score, double remainingTime);

    // ゲーム終了通知の表示
    void showGameOver();

    // コンボ終了時の結果表示
    void showComboResult(int score, int comboCount);

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
    int m_displayComboScore = 0;            // コンボ期間中の総スコア
    ComboScoreInfo m_latestComboScore = {}; // 最新のコンボ得点情報

    // コンボ表示用のフォント
    Font m_comboFont{40, Typeface::Bold};
    Font m_multiplierFont{24};
    Font m_scoreFont{32, Typeface::Bold};

    // ゲーム情報表示用
    int m_displayScore = 0;          // 表示するスコア
    double m_displayRemainingGameTime = 100.0; // 表示する残り時間（初期値100秒）
    Font m_gameInfoFont{40, Typeface::Bold};   // フォントサイズを大きく

    // ゲーム終了通知用
    bool m_showGameOver = false;     // ゲーム終了通知を表示するか
    Font m_gameOverFont{60, Typeface::Bold};

    // コンボ終了結果の表示用
    int m_comboResultScore = 0;
    int m_comboResultCount = 0;
    Stopwatch m_comboResultTimer;
    static constexpr double m_comboResultDuration = 2.0; // 2秒間表示
};
