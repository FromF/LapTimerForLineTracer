/*
  IRブレークビームによる区間時間計測サンプルコード
  
  条件:
  - スタートライン用センサー: デジタルピン2
  - ゴールライン用センサー: デジタルピン3
  - 内部プルアップ利用: センサー出力はLOW=ビーム通過、HIGH=遮断(仮)
  
  出力フォーマット(CSV):
  current_time, previous_time, difference
  
  動作:
  - スタートラインが遮断された瞬間にstart_timeを記録。
  - その後、ゴールラインが遮断された瞬間にend_timeを記録し、(end_time - start_time)で区間時間取得。
  - 次の計測時にはprevious_timeとして前回データを利用し、差分を出力。
  - ライン外れなどで再走行する場合は、再びスタートライン通過が起点になるため、スタートライン通過前には計測をリセット。
*/
#include <M5StickC.h>

unsigned long start_time = 0; 
unsigned long end_time = 0; 
unsigned long previous_time = 0; 
unsigned long current_time = 0;

bool start_triggered = false;
bool end_triggered = false;

const int startSensorPin = 2;
const int endSensorPin   = 3;

// センサーの状態を監視するための変数
int lastStartState = HIGH; // pullupでHIGH、遮断でLOWを想定の場合は適宜変更
int lastEndState   = HIGH;

void setup() {
  Serial.begin(115200);
  
  // pinMode(startSensorPin, INPUT_PULLUP);
  // pinMode(endSensorPin, INPUT_PULLUP);
  M5.begin();  // Initialize the M5StickC object.
  
  // 初期状態のセンサー読み取り
  // lastStartState = digitalRead(startSensorPin);
  // lastEndState   = digitalRead(endSensorPin);
  lastStartState = HIGH;
  lastEndState   = HIGH;
  
  // 初期ヘッダ出力(任意)
  M5.Lcd.setTextColor(
      YELLOW);  // Set the font color to yellow.  设置字体颜色为黄色
  M5.Lcd.setRotation(3);
  M5.Lcd.println("Mesure Simulator");
  M5.Lcd.setTextColor(RED);
}

void loop() {
  // int currentStartState = digitalRead(startSensorPin);
  // int currentEndState   = digitalRead(endSensorPin);
  int currentStartState = HIGH;
  int currentEndState = HIGH;
  M5.update();
  if (M5.BtnA.wasReleased()) {
    lastStartState = LOW;
  }
  if (M5.BtnB.wasReleased()) {
    lastEndState = LOW;
  }
  
  // スタートライン通過検出(立ち下がりor立ち上がりで判定)
  // ここでは、センサー出力が光遮断でHIGHになると仮定し、
  // 「LOW->HIGH」の変化で通過とみなす。実際にはセンサー仕様を確認。
  if (lastStartState == LOW && currentStartState == HIGH) {
    // スタートラインを通過した瞬間
    start_time = micros(); // microseconds単位、必要ならmillis()へ変更
    start_triggered = true;
    end_triggered = false;
    Serial.println("start");
    M5.Lcd.println("start");
  }

  // ゴールライン通過検出
  if (start_triggered && !end_triggered) {
    if (lastEndState == LOW && currentEndState == HIGH) {
      // ゴールラインを通過した瞬間
      end_time = micros();
      end_triggered = true;
      
      // 計測結果算出
      current_time = end_time - start_time; // マイクロ秒単位の計測時間
      // 必要に応じてミリ秒変換
      // double current_ms = (double)current_time / 1000.0;
      
      // CSV出力
      // long difference = (previous_time == 0) ? 0 : (current_time - previous_time);
      // Serial.print(current_time);
      // Serial.print(",");
      // Serial.print(previous_time);
      // Serial.print(",");
      // Serial.println(difference);
      Serial.print("goal,");
      Serial.println(current_time);
      M5.Lcd.print("goal,");
      M5.Lcd.println(current_time);
      
      // 前回時間を更新
      previous_time = current_time;
      
      // 次回計測に備えてフラグリセット（ライン外れなどで再走行する場合は、改めてスタートライン通過が必要）
      start_triggered = false;
    }
  }
  
  lastStartState = currentStartState;
  lastEndState   = currentEndState;
}