/*
  IR ブレークビームを想定した区間時間計測サンプルコード (M5StickC 用)

  【概要】
   - 本コードでは、本来デジタルピン 2（スタートライン）、3（ゴールライン）に接続する
     IR ブレークビームセンサーを想定しています。
   - しかし、M5StickC 上で実行するために、ここではセンサー入力を M5StickC のボタンで
     シミュレートしています。（BtnA → スタートライン通過、BtnB → ゴールライン通過）
   - スタートラインを通過した時点で start_time（micros）を記録し、
     ゴールライン通過時点で end_time を記録して計測区間の時間を求めます。
   - 計測結果は Serial モニタと M5StickC の LCD に表示されます。
   - 実際の IR センサーを使用する場合は、該当箇所の pinMode() や digitalRead() を有効にし、
     センサーの出力ロジックに合わせて検出タイミングを調整してください。

  【センサー想定仕様】（仮）
   - デジタルピン 2：スタートセンサー
   - デジタルピン 3：ゴールセンサー
   - INPUT_PULLUP 利用時：
     センサー出力はビームが「通過」すると LOW、ビームが「遮断」されている時は HIGH。
     （実際のセンサー仕様によってはこの限りではありません）

  【計測の流れ】
   1) スタートライン（BtnA）を通過した瞬間に start_time = micros() を記録。
   2) ゴールライン（BtnB）を通過した瞬間に end_time = micros() を記録。
   3) (end_time - start_time) で区間時間を算出して表示。
   4) 次の計測を行うには、再度スタートラインを通過する（BtnA を押す）ところから始まります。

  【注意事項】
   - ボタンA、B は 「wasReleased()」で押下を判定しています。
   - 本コードはサンプルであり、実際の競技や計測に用いる場合は誤差や環境ノイズを考慮して
     処理を調整してください。

  【出力例 (シリアル/LCD)】
   - "start" （スタートライン通過検出）
   - "goal,123456" （ゴールライン到達時の区間時間 microseconds 単位）
*/

#include <M5StickC.h>

unsigned long start_time = 0; 
unsigned long end_time   = 0; 
unsigned long current_time = 0;

// スタート・ゴールラインが通過したかを管理するフラグ
bool start_triggered = false;
bool end_triggered   = false;

// 実機センサー使用時のピン設定（現在はコメントアウト）
const int startSensorPin = 2;  // スタートセンサー用
const int endSensorPin   = 3;  // ゴールセンサー用

// センサーの前回状態を保持するための変数
// pullup で HIGH、ビーム通過で LOW に変化する場合を想定（実機に合わせて要修正）
int lastStartState = HIGH;
int lastEndState   = HIGH;

void setup() {
  Serial.begin(115200);
  M5.begin();  // M5StickC の初期化
  
  // 実機センサー使用時は以下を有効にする
  // pinMode(startSensorPin, INPUT_PULLUP);
  // pinMode(endSensorPin, INPUT_PULLUP);
  //
  // lastStartState = digitalRead(startSensorPin);
  // lastEndState   = digitalRead(endSensorPin);

  // LCD の簡易表示設定
  M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.setRotation(3);
  M5.Lcd.println("Measure Simulator");
  M5.Lcd.setTextColor(RED);
}

void loop() {
  // 実機センサー使用時は以下を有効にし、現状のセンサー出力を読む
  // int currentStartState = digitalRead(startSensorPin);
  // int currentEndState   = digitalRead(endSensorPin);
  //
  // ここでは M5StickC のボタン押下をセンサー通過に見立てる
  int currentStartState = HIGH;
  int currentEndState   = HIGH;

  // M5StickC のイベント更新
  M5.update();

  // ButtonA を離した瞬間をスタートライン通過に見立てる
  if (M5.BtnA.wasReleased()) {
    lastStartState = LOW;
  }
  // ButtonB を離した瞬間をゴールライン通過に見立てる
  if (M5.BtnB.wasReleased()) {
    lastEndState = LOW;
  }

  // スタートライン通過検出
  // 例：センサー出力が「LOW→HIGH」へ変化したとき通過とみなす（実際のセンサー仕様に応じて変更）
  if (lastStartState == LOW && currentStartState == HIGH) {
    // スタートライン通過時刻を記録
    start_time = micros();
    start_triggered = true;
    end_triggered   = false;

    Serial.println("start");
    M5.Lcd.println("start");
  }

  // ゴールライン通過検出
  if (start_triggered && !end_triggered) {
    if (lastEndState == LOW && currentEndState == HIGH) {
      // ゴールライン通過時刻を記録
      end_time = micros();
      end_triggered = true;
      
      // 計測結果算出 (マイクロ秒単位)
      current_time = end_time - start_time;
      Serial.print("goal,");
      Serial.println(current_time);
      M5.Lcd.print("goal,");
      M5.Lcd.println(current_time);
      
      // 次回計測に備えてフラグリセット（ライン外れなどで再走行する場合は、改めてスタートライン通過が必要）
      start_triggered = false;
    }
  }

  // センサー状態を更新
  lastStartState = currentStartState;
  lastEndState   = currentEndState;
}