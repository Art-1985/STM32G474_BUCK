# 專案進度

最後更新：2026-08-07

## 目前狀態

目前完成「文件與架構基線」，尚未執行韌體修改、Keil 全量建置或板上量測。

| 工作流 | 狀態 | 證據/備註 |
|---|---|---|
| 倉庫與 6 個例程盤點 | 完成 | 已辨識單環與雙環兩類架構 |
| UML 與狀態機 | 完成 | [docs/UML.md](docs/UML.md) |
| 應用呼叫圖 | 完成 | [docs/CALL_GRAPH.md](docs/CALL_GRAPH.md) |
| C4 架構模型 | 完成 | [docs/C4.md](docs/C4.md) |
| 例程 1～3 電壓版本比較 | 完成 | [docs/EXAMPLES_1_3_VOLTAGE_VARIANT_COMPARISON.md](docs/EXAMPLES_1_3_VOLTAGE_VARIANT_COMPARISON.md) |
| 根目錄架構入口 | 完成 | [ARCHITECTURE.md](ARCHITECTURE.md) |
| 專案治理文件 | 完成 | `AGENTS/PLAN/PROGRESS/DECISIONS/TASKS/MEMORY/CHANGELOG` |
| Mermaid 語法驗證 | 完成 | 根目錄與 `docs/` 共 14 個圖表已由 Mermaid CLI 實際渲染 |
| Markdown 基本檢查 | 完成 | 相對連結、圍欄、尾端空白均已檢查 |
| 6 套 Keil 工程建置 | 尚未開始 | 尚未確認本機 Keil 命令列環境 |
| 即時 ISR 時序量測 | 尚未開始 | 需要目標板與量測設備 |
| PWM/ADC 相位驗證 | 尚未開始 | 需要板測或至少參考手冊逐項確認 |
| 韌體風險修正 | 尚未開始 | 本輪只新增文件 |

## 已確認的主要結果

- `main()` 依序呼叫 `Initial_prepheral_()` 與永不返回的 `state_machine()`。
- HRTIM Timer A 產生 TA1/TA2 PWM、CMP3 ADC 觸發及 REP 中斷。
- REP ISR 負責 ADC 工程量換算、PI 更新與軟啟動參考值遞增；前台狀態機負責 OVP/UVP/OCP 與輸出門控。
- 例程 1～3 為電壓單環；例程 4～6 為電壓/電流雙環競爭。
- 例程 1～3 的 256 個相對路徑檔案中，249 個逐位元組相同；功能差異不只 `Target_voltage`，還包含輸出 OVP、PI 上限／係數、REP 更新率及一項待確認的 `DISdriver` 啟動漂移。
- 目前所有 HRTIM fault 輸入停用，保護主要是前台軟體路徑。

## 已識別但尚未修正

- ISR/前台共享變數缺少明確的 `volatile`/同步策略。
- `ADC2_RESULT` 的定義與外部宣告尺寸不一致。
- 單環與雙環例程的 OCP 計數恢復語意不同。
- 1 秒忙等待會暫停前台保護掃描。
- ADC 外部觸發與連續轉換同時啟用，實際取樣相位需要驗證。
- `common.h` 造成大範圍循環 include，部分宣告與變數未使用。

## 下一個建議檢查點

先完成 [TASKS.md](TASKS.md) 的 B-001～B-003：確認 6 個 Keil target、逐一建置並記錄工具鏈/警告。建置基線完成前，不建議進行跨 6 個例程的共用程式碼重構。
