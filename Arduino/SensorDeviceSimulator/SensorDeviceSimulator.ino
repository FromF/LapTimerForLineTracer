/*
  IR ブレークビームを想定した区間時間計測サンプルコード (Arduino Nano Every 用)

  【概要】
   - デジタルピン 2（スタートライン）、3（ゴールライン）に接続された
     IR ブレークビームセンサーを想定し、区間通過時間を計測します。
   - スタートラインを通過した時点で start_time（micros()）を記録し、
     ゴールラインを通過した時点で end_time（micros()）を記録して、
     区間の経過時間を算出します。
   - 計測結果はシリアルモニタ（115200 bps）に出力します。

  【センサー想定仕様】（例）
   - デジタルピン 2：スタートセンサー
   - デジタルピン 3：ゴールセンサー
   - INPUT_PULLUP を使用し、通常時は HIGH、ビームが通過すると LOW になる想定。
     （実際のセンサー仕様によって動作を変更してください）

  【計測の流れ】
   1) スタートラインを通過（センサー出力が LOW → HIGH に変化）した瞬間、
      start_time = micros() を記録。
   2) ゴールラインを通過（センサー出力が LOW → HIGH に変化）した瞬間、
      end_time = micros() を記録。
   3) (end_time - start_time) で区間時間を求め、シリアルモニタに表示する。
   4) 次の計測時は再度スタートラインの通過から始まる。

  【注意事項】
   - micros() 関数はマイクロ秒単位で時間を返しますが、誤差やオーバーフローに注意してください。
   - センサーのロジック（HIGH/LOW のタイミング）は機種によって異なる場合があるため、
     必要に応じて変更してください。
   - 実際の競技や試験で使用する場合は環境ノイズ等を考慮し、ハードウェアやソフトウェア的な
     デバウンスやフィルタなどを検討してください。
*/

unsigned long start_time = 0; 
unsigned long end_time   = 0; 
unsigned long current_time = 0;

// スタート・ゴールラインが通過したかを管理するフラグ
bool start_triggered = false;
bool end_triggered   = false;

// ピン設定
const int startSensorPin = 2;  // スタートセンサー
const int endSensorPin   = 3;  // ゴールセンサー

// センサーの前回状態を保持するための変数
// pullup で HIGH、ビーム通過で LOW に変化する場合を想定（実機に合わせて要修正）
int lastStartState = HIGH;
int lastEndState   = HIGH;

void setup() {
  Serial.begin(115200);

  // 入力ピンをプルアップに設定
  pinMode(startSensorPin, INPUT_PULLUP);
  pinMode(endSensorPin, INPUT_PULLUP);

  // 初期状態を取得
  lastStartState = digitalRead(startSensorPin);
  lastEndState   = digitalRead(endSensorPin);
}

void loop() {
  // 現在のセンサー状態を読み取り
  int currentStartState = digitalRead(startSensorPin);
  int currentEndState   = digitalRead(endSensorPin);

  // スタートライン通過検出
  // 例：LOW → HIGH の立ち上がりで通過とみなす（センサー仕様に応じて変更）
  if (lastStartState == LOW && currentStartState == HIGH) {
    start_time = micros();
    start_triggered = true;
    end_triggered   = false;
    Serial.println("start");
  }

  // ゴールライン通過検出
  if (start_triggered && !end_triggered) {
    if (lastEndState == LOW && currentEndState == HIGH) {
      end_time = micros();
      end_triggered = true;
      
      // 計測結果算出 (マイクロ秒単位)
      current_time = end_time - start_time;
      Serial.print("goal,");
      Serial.println(current_time);
      
      // 次回計測に備えてフラグリセット（ライン外れなどで再走行する場合は、改めてスタートライン通過が必要）
      start_triggered = false;
    }
  }

  // センサー状態を更新
  lastStartState = currentStartState;
  lastEndState   = currentEndState;
}