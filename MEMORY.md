# 專案記憶

本文件保存跨工作階段仍然有效的事實、陷阱與導航資訊。計畫看 [PLAN.md](PLAN.md)，可執行任務看 [TASKS.md](TASKS.md)，具約束力的選擇看 [DECISIONS.md](DECISIONS.md)。

## 穩定事實

- 倉庫包含 6 套獨立 STM32G474 Keil 工程。
- 例程 1～3：800 kHz 電壓單閉環，`PWM_PERIOD = 6800`。
- 例程 4～6：200 kHz 電壓/輸出電流雙 PI，`PWM_PERIOD = 27200`。
- 目標輸出依例程為 5 V、12 V、16 V；雙環例程另設 3 A 目標電流。
- 所有例程的輸入 OVP/UVP 設為 24 V/8 V，輸出 OCP 設為 3.6 A；輸出 OVP 隨版本不同。
- 應用主路徑是 `main -> Initial_prepheral_ -> state_machine`，狀態機永不返回。

## 即時執行模型

- `Initial_prepheral_()` 的順序：HAL、HSE 系統時鐘、TIM16、TIM2、LED GPIO、ADC1、ADC2、HRTIM。
- ADC1 掃描 Iin/Vin/IL 三路到 `ADC1_RESULT[3]`。
- ADC2 掃描 Iout/Vout 兩路到 `ADC2_RESULT[2]`。
- HRTIM Timer A CMP3 被設定為 ADC Trigger 1；ADC1/2 都選用此外部觸發。
- HRTIM Timer A REP 中斷入口是 `HRTIM1_TIMA_IRQHandler()`，實作位於 `mycode/state_machine.c`。
- ISR 清除標誌、置 `Data_update_flag`、換算工程量，輸出啟動後呼叫 PI，再更新 CMP1（脈寬）與 CMP3（脈寬中點）。
- 前台狀態機消費 `Data_update_flag`，依序檢查 Vin、Vout、Iout，首次正常時等待 1 秒後使能驅動與 TA1/TA2。
- 故障回到初始狀態，`Reset_VAR()` 同時使用 PA11 `DISdriver` 與 HRTIM `ODISR` 關閉輸出。

## 例程差異

| 例程 | 模式 | REP | 推算 ISR 頻率 | PI 介面 |
|---|---|---:|---:|---|
| 1 | 5 V 單環 | 9 | 80 kHz | `PID_loop(Vout)` |
| 2 | 12 V 單環 | 9 | 80 kHz | `PID_loop(Vout)` |
| 3 | 16 V 單環 | 5 | 約 133.3 kHz | `PID_loop(Vout)` |
| 4～6 | 5/12/16 V、3 A 雙環 | 2 | 約 66.7 kHz | `PID_loop(Vout, Iout)` |

REP 頻率按 `fPWM / (REP + 1)` 推算，仍需板上量測確認。

- 例程 1～3 共比較 256 個相對路徑檔案，249 個逐位元組相同；7 個不同檔案中，控制行為差異集中於 `state_machine.c`、`PID.c`、`HRTIM.c`。詳細證據見 [docs/EXAMPLES_1_3_VOLTAGE_VARIANT_COMPARISON.md](docs/EXAMPLES_1_3_VOLTAGE_VARIANT_COMPARISON.md)。
- 例程 1 的 PI duty 上限約 50%，例程 2/3 約 65%；例程 3 另修改 PI 係數與 REP 更新率。
- `PID_INT()` 的 0.01／0.015 係數只產生約 20／30 count 的 `Pulse_width` RAM 初值，沒有直接寫入 HRTIM；第一次 `PID_loop()` 會在寫 CMP 前覆寫它，不應誤當成有效起始 duty。
- 例程 1 在 `HRTIM_INT()` 額外執行 `DISdriver`，例程 2/3 沒有；三者稍後仍都由 `Reset_VAR()` 執行 `DISdriver + ODISR`。這是待板測的啟動安全漂移，不是已證實的電壓需求。

## 已知陷阱

- ISR 與前台共享的旗標、量測值及參考值沒有完整的 `volatile`/一致性設計。
- `ADC2_RESULT` 定義為 `[2]`，但 `state_machine.c` 外部宣告為 `[3]`。
- 單環例程的 OCP 計數不在正常電流時清零；雙環例程會清零。
- `Delay_ms(1000)` 期間中斷仍執行，但前台保護不掃描，PI 又因 `flag_start_cnt` 尚未清除而不執行。
- ADC 同時啟用外部觸發與連續轉換；不可未經驗證就假定每次 CMP3 僅對應一次同步取樣。
- HRTIM Timer A 的 fault input 目前停用，COMP2/DAC 註解不等於已有活動硬體保護。
- `common.h` 幾乎包含所有模組，而模組頭檔又包含 `common.h`，形成循環 include。
- 原始 C/H 註解存在非 UTF-8 編碼；避免整檔格式化或編碼轉換。

## 導航

- 穩定架構入口：[ARCHITECTURE.md](ARCHITECTURE.md)
- C4 系統邊界、容器、元件與部署：[docs/C4.md](docs/C4.md)
- 例程 1～3 輸出電壓版本逐檔比較：[docs/EXAMPLES_1_3_VOLTAGE_VARIANT_COMPARISON.md](docs/EXAMPLES_1_3_VOLTAGE_VARIANT_COMPARISON.md)
- UML、狀態機、時序、資料流：[docs/UML.md](docs/UML.md)
- 函式與硬體事件呼叫圖：[docs/CALL_GRAPH.md](docs/CALL_GRAPH.md)
- 當前進度：[PROGRESS.md](PROGRESS.md)
- 歷史變更：[CHANGELOG.md](CHANGELOG.md)
