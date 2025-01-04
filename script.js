/* ---------------------------------
   Web Serial API まわりの変数と設定
----------------------------------- */
let port;
let reader;
let inputStream;
let outputStream;
let inputDone;
let outputDone;

/* ---------------------------------
   計測まわりの変数
----------------------------------- */
let isRunning = false;      // ストップウォッチ計測中かどうか
let startTime = 0;          // 計測開始時刻 (Date.now())
let timerId = null;         // setIntervalのID
let bestLapTime = Infinity; // ベストラップ(ミリ秒)
let laps = [];              // ラップタイムのリスト(ミリ秒)

// 要素の取得
const connectBtn   = document.getElementById('connectBtn');
const clearLapsBtn = document.getElementById('clearLapsBtn');
const stopwatchEl  = document.getElementById('stopwatch');
const bestLapEl    = document.getElementById('bestLap');
const lapListEl    = document.getElementById('lapList');

/* ---------------------------------
   ボタンのイベント設定
----------------------------------- */
// デバイスと接続
connectBtn.addEventListener('click', async () => {
  try {
    // シリアルポート取得
    port = await navigator.serial.requestPort();
    // ボーレートは115200に設定
    await port.open({ baudRate: 115200 });

    // 入力ストリーム設定
    const textDecoder = new TextDecoderStream();
    inputDone = port.readable.pipeTo(textDecoder.writable);
    inputStream = textDecoder.readable.pipeThrough(
      new TransformStream(new LineBreakTransformer())
    );
    reader = inputStream.getReader();

    // 出力ストリーム設定
    const textEncoder = new TextEncoderStream();
    outputDone = textEncoder.readable.pipeTo(port.writable);
    outputStream = textEncoder.writable;

    // 受信ループ開始
    readLoop();

    console.log("Serial connected");
    alert("デバイスに接続しました");
  } catch (err) {
    console.error("Connection error:", err);
    alert("接続失敗: " + err.message);
  }
});

// ラップリストをクリア
clearLapsBtn.addEventListener('click', () => {
  laps = [];
  lapListEl.textContent = "";
  bestLapTime = Infinity;
  bestLapEl.textContent = "--:--.---";
});

/* ---------------------------------
   シリアル受信処理
----------------------------------- */
async function readLoop() {
  try {
    while (true) {
      const { value, done } = await reader.read();
      if (done) {
        // 読み取り完了
        console.log("[readLoop] Stream closed.");
        break;
      }
      if (value) {
        handleDeviceMessage(value.trim());
      }
    }
  } catch (err) {
    console.error("Read error:", err);
  }
}

/**
 * デバイスからのメッセージ("start" or "goal,xxx")を解析して処理する
 * - "start": スタート地点通過
 * - "goal,xxxx": ゴール地点通過。カンマ以降は区間時間(μs)
 */
function handleDeviceMessage(msg) {
  if (msg === "start") {
    // もし既に走行中ならゴールしないまま再度スタートが来た、ということなので
    // ラップ記録しない (recordLap=false) でストップウォッチを停止
    if (isRunning) {
      stopStopwatch(undefined, false); 
    }
    startStopwatch();
  } else if (msg.startsWith("goal,")) {
    const parts = msg.split(",");
    if (parts.length === 2) {
      // カンマ以降は区間時間(マイクロ秒: μs)なので
      // ミリ秒に変換する (1 ms = 1000 μs)
      const intervalUs = parseInt(parts[1], 10); // μs
      const intervalMs = intervalUs / 1000;      // ms
      // ゴールが確定したタイムなのでラップ記録する (recordLap=true)
      stopStopwatch(intervalMs, true);
    }
  }
}

/* ---------------------------------
   ストップウォッチロジック
----------------------------------- */

/**
 * ストップウォッチ開始
 */
function startStopwatch() {
  isRunning = true;
  startTime = Date.now();

  // 画面表示をリセット
  stopwatchEl.textContent = formatTime(0);

  // 100msごとに現在の経過時間を表示更新
  timerId = setInterval(() => {
    const elapsed = Date.now() - startTime;
    stopwatchEl.textContent = formatTime(elapsed);
  }, 100);
}

/**
 * ストップウォッチ終了
 * @param {number|undefined} goalMs ゴール時間(ミリ秒) 省略時は Date.now() - startTime
 * @param {boolean} recordLap ラップタイムとして記録するかどうか (true/false)
 */
function stopStopwatch(goalMs, recordLap = true) {
  if (!isRunning) return;

  clearInterval(timerId);
  timerId = null;
  isRunning = false;

  let finalMs;
  if (typeof goalMs === "number") {
    finalMs = goalMs;
  } else {
    finalMs = Date.now() - startTime;
  }

  // ストップウォッチ表示をゴールタイムに固定
  stopwatchEl.textContent = formatTime(finalMs);

  // recordLap=falseの場合、ここでリターンするかどうかは好みで決定
  if (!recordLap) {
    return;
  }

  // ラップ記録する場合のみ以下を実行

  // ベストラップ更新チェック
  if (finalMs < bestLapTime) {
    bestLapTime = finalMs;
    bestLapEl.textContent = formatTime(bestLapTime);
  }

  // ラップリストに記録
  laps.push(finalMs);
  appendLapTime(finalMs);
}

/**
 * ラップタイム一覧に1行追加
 */
function appendLapTime(ms) {
  const newLine = formatTime(ms);
  lapListEl.textContent += newLine + "\n";
  // スクロールを末尾に合わせる
  lapListEl.scrollTop = lapListEl.scrollHeight;
}

/**
 * mm:ss.000 形式にフォーマットする関数
 */
function formatTime(milliseconds) {
  const ms = Math.floor(milliseconds) % 1000;
  const totalSec = Math.floor(milliseconds / 1000);
  const sec = totalSec % 60;
  const min = Math.floor(totalSec / 60);

  const mm = String(min).padStart(2, '0');
  const ss = String(sec).padStart(2, '0');
  const mmm = String(ms).padStart(3, '0');

  return `${mm}:${ss}.${mmm}`;
}

/* ---------------------------------
   TransformStream用クラス
   (改行区切りで分割する例)
----------------------------------- */
class LineBreakTransformer {
  constructor() {
    this.container = '';
  }
  transform(chunk, controller) {
    this.container += chunk;
    const lines = this.container.split('\n');
    this.container = lines.pop(); // 最後に残った行未満の文字列を保持
    for (const line of lines) {
      controller.enqueue(line);
    }
  }
  flush(controller) {
    if (this.container) {
      controller.enqueue(this.container);
    }
  }
}