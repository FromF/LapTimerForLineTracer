# Lap Timer for Line Tracer.
ライントレーサー用ラップタイマー

# 動作環境
- Web Serial APIを使っているので、Chrome 89以降や一部のChromium系ブラウザ

# 仕様

## 1. Webアプリの目的
- シリアル通信で受信した “start” や “goal,xxxx” メッセージに応じて、区間時間を計測・表示する。  
- 計測したラップタイムをリスト表示し、コピー・クリアなどを行うことができる。

---

## 2. 画面構成

1. **ストップウォッチ表示エリア**  
   - (mm:ss.000) の形式で表示。  
   - “start” メッセージを受け取ると 0 から計測を開始。  
   - 計測中はリアルタイムで経過時間を更新。  
   - “goal,xxxx” を受信したら、最終的なタイムを確定表示する。

2. **ベストラップ表示エリア**  
   - (mm:ss.000) の形式。  
   - ゴールが確定したタイムが過去の記録より小さい場合、この表示を更新する。  
   - 「--:--.---」で初期化しておき、ラップリストクリア時にもリセットする。

3. **全ラップタイム一覧エリア**  
   - すべてのゴール時のタイムを最終行に追加（改行区切り）。  
   - マウス範囲選択 & コピーを想定し、テキストとして表示する。  
   - クリアボタンで一覧を消去する。

4. **ボタン類**  
   - **Connect**: シリアル接続ボタン。押下するとポート選択ダイアログを開いて対象デバイスと接続する。  
   - **Clear Lap List**: ラップタイム一覧とベストラップをリセットするボタン。

---

## 3. シリアル通信

- **Web Serial API** を使用。  
- 対応ブラウザ(Chromium系など)で、HTTPSまたは`localhost`環境が必要。  
- ボーレートは **115200** に設定。  
- ストリームを `TextDecoderStream` + `TransformStream` で行単位に分割して受信する。  
- 受信メッセージによりアプリの動作を変化させる。

---

## 4. 受信メッセージの形式と動作

1. **"start"**  
   - 「スタート地点通過」を示す。  
   - すでに計測中の場合(ゴールが来ていないのに再度スタートが届いた場合)は、**前の計測を破棄**して新規にスタートする。破棄した計測はラップリストに追加しない。  
   - 計測中でない場合は、新規スタートとしてストップウォッチを 0 から開始。

2. **"goal,xxxx"**  
   - 「ゴール地点通過」を示す。カンマ以降 (`xxxx`) は**区間時間(マイクロ秒, μs)**。  
   - このタイムをミリ秒に換算(1ms = 1000μs)してストップウォッチを確定表示し、**ベストラップの更新確認**・**ラップリストへの追加**を行う。

---

## 5. 動作の詳細

1. **スタート処理**  
   - `isRunning` フラグを立て、`startTime = Date.now()` で現在時刻を記録。  
   - 一定間隔(100ms)で、経過時間(現在時刻 - startTime)をストップウォッチ領域に反映するタイマーを起動。

2. **ゴール処理**  
   - 受け取ったマイクロ秒をミリ秒に変換して最終時間を確定。  
   - タイマーを停止し、ストップウォッチ表示を確定タイムに更新。  
   - ベストラップより小さければベストラップを更新。  
   - ラップリストの末尾に今回のタイムを追記。

3. **ゴールせず再スタート**  
   - 現在の計測を停止(ラップリストに追加しない) → 新規にスタート。

4. **クリア処理**  
   - ラップリスト配列・表示を空にし、ベストラップを `Infinity` (表示は “--:--.---”) に戻す。

---

## 6. フォーマット

- **時間表示**: (mm:ss.000)  
  - mm: 2桁  
  - ss: 2桁  
  - その後に “.” で3桁のミリ秒。

- **マイクロ秒(μs) → ミリ秒(ms)**  
  - `intervalMs = intervalUs / 1000;`

---

## 7. その他注意事項

- 実運用ではエラーハンドリング、ポート接続の再試行などを必要に応じて追加する。  
- Web Serial API は一部ブラウザ・環境にしか対応していない点に留意。  
- セキュアコンテキスト (HTTPS or `localhost`) でのみ使用可能。
