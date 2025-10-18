
# siv3d_bullet

このディレクトリは、Siv3DとBullet物理エンジンを組み合わせた動作確認用の練習コードです。

## プロジェクトの説明
- Bullet物理エンジンを使った物理シミュレーションのサンプルです。
- Siv3Dの描画機能とBulletの物理演算を組み合わせています。
- 主なファイル：
  - `Main.cpp` : エントリポイント
  - `PhysicsWorld.*` : 物理世界の管理
  - `PhysicsObject.*` : 物理オブジェクトの定義
- Siv3Dの型（例：Vec3など）は表に出し、Bulletは内部処理で利用しています。

## Visual Studioでのビルド方法
[Qiita記事](https://qiita.com/23tas9/items/522312d04013f3512f33)が参考になりますが、一部変更点があります。

基本的な手順は以下の通りです。
1. Visual Studioで「表示」→「その他のウィンドウ」→「プロパティマネージャー」を開きます。
2. 新しいプロパティシートを追加します。
3. Bulletのパスをプロパティシートに追加します（環境によってパスが異なるので、別ファイルで設定します）。
  - C/C++ → 全般 → 追加のインクルードディレクトリに `bullet/src` を追加
  - リンカー → 全般 → 追加のライブラリディレクトリに `bullet/bin` を追加
4. C/C++ → プリプロセッサ → プリプロセッサの定義に以下を追加します。
  ```
  BT_THREADSAFE=1
  BT_USE_DOUBLE_PRECISION
  ```
5. Debug構成の「リンカー」→「入力」→「追加の依存ファイル」に以下を追加します。
  ```
  BulletCollision_vs2010_x64_debug.lib
  BulletDynamics_vs2010_x64_debug.lib
  BulletSoftBody_vs2010_x64_debug.lib
  LinearMath_vs2010_x64_debug.lib
  ```
6. Release構成の「リンカー」→「入力」→「追加の依存ファイル」に以下を追加します。
  ```
  BulletCollision_vs2010_x64_release.lib
  BulletDynamics_vs2010_x64_release.lib
  BulletSoftBody_vs2010_x64_release.lib
  LinearMath_vs2010_x64_release.lib
  ```

