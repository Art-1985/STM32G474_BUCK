# 例程 1～3 輸出電壓版本差異分析

## 結論先行

例程 1（5 V）、例程 2（12 V）與例程 3（16 V）不是只修改 `Target_voltage`。三套工程的 256 個相對路徑檔案中，249 個檔案逐位元組相同，只有 7 個檔案不同；其中會改變韌體行為的差異集中在：

- `state_machine.c`：目標電壓與輸出 OVP 門檻。
- `PID.c`：PI 係數、PI 輸出上限，以及一個實際未寫入 HRTIM 的 `Pulse_width` 初始化值。
- `HRTIM.c`：REP 中斷分頻；另有一項無法由輸出電壓合理解釋的 `DISdriver` 啟動差異。

因此，若要建立另一個輸出電壓版本，至少要共同審查目標值、OVP、PI 係數、占空比上限與控制更新率；不能只複製工程後改一個電壓常數。

本文是靜態原始碼與檔案雜湊比較，未重新建置 Keil 工程，也未執行板上量測。文中的 REP 頻率、軟啟動時間與保護計數時間均為依程式推算值。

## 比較對象與方法

為縮短後文，使用下列代稱：

| 代稱 | 工程 | 名義輸出 |
|---|---|---:|
| E1 | `例程1_左边12V输入右边5V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护` | 5 V / 3 A |
| E2 | `例程2_左边12V输入右边12V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护` | 12 V / 3 A |
| E3 | `例程3_左边12V输入右边16V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护` | 16 V / 3 A |

比較方式如下：

1. 以相對路徑建立三套工程的檔案聯集。
2. 對每個檔案計算 SHA-256，先找出逐位元組不同的檔案。
3. 對應用程式差異逐行比較，再追蹤變數的讀取、寫入與硬體暫存器效果。
4. 將 Keil 個人設定與產生檔和實際控制邏輯分開分類。

此方法能確認倉庫目前保存的差異，不能證明所有差異都是原作者有意為之，也不能取代電源級模型、元件額定值、補償設計與板測。

## 參數總表

| 項目 | E1：5 V | E2：12 V | E3：16 V | 實際作用 |
|---|---:|---:|---:|---|
| `Target_voltage` | 5.0 | 12.0 | 16.0 | 軟啟動最終參考值 |
| `Vout_ovp` | 10.0 | 15.0 | 19.0 | 前台輸出過壓門檻 |
| `V_PI.Kp` | 0.01 | 0.01 | 0.015 | 增量式 PI 係數 |
| `V_PI.Ki` | 0.001 | 0.001 | 0.002 | 增量式 PI 係數 |
| `V_PI.error_add` 上限 | 1.65 | 2.145 | 2.145 | 約 50% / 65% / 65% 最大占空比 |
| `V_PI.error_add` 下限 | 0.05 | 0.05 | 0.05 | 約 1.515% 最小占空比 |
| `PID_INT()` 的初始 `Pulse_width` 係數 | 0.015 | 0.01 | 0.015 | 僅改 RAM 變數；目前路徑不直接套用至 PWM |
| `PWM_PERIOD` | 6800 | 6800 | 6800 | 三者開關週期相同 |
| `RepetitionCounter` | 9 | 9 | 5 | 改變 REP ISR 與控制更新率 |
| 推算 REP ISR 頻率 | 80 kHz | 80 kHz | 約 133.3 kHz | `800 kHz / (REP + 1)` |
| `Vin_ovp` / `Vin_uvp` | 24 / 8 V | 24 / 8 V | 24 / 8 V | 相同 |
| `Iout_ocp` | 3.6 A | 3.6 A | 3.6 A | 相同 |
| `HRTIM_INT()` 內 `DISdriver` | 有 | 無 | 無 | 啟動關斷時序差異，不能視為電壓參數 |

### 由程式可直接推算的時間差異

ISR 每次在軟啟動期間執行 `Vref += 0.001f`，所以理想斜率為：

```text
Vref 斜率 = 0.001 V/tick × REP tick/s
E1、E2：0.001 × 80,000       = 80 V/s
E3：    0.001 × 133,333.3... ≈ 133.3 V/s
```

不含前台固定的 1 秒啟動等待，從 0 V 參考值升至名義目標的推算時間為：

| 工程 | 推算參考值爬升時間 |
|---|---:|
| E1：5 V | 約 62.5 ms |
| E2：12 V | 約 150 ms |
| E3：16 V | 約 120 ms |

前台 OVP 在 `OVP_CNT > 200` 時回到初始狀態；若前台沒有漏掉 `Data_update_flag`，相當於第 201 次連續越限才觸發。推算最短連續越限時間約為 E1/E2 的 2.51 ms、E3 的 1.51 ms。OCP 使用 `OCP_CNT1 > 2000`，而單環版本正常電流時沒有清零，故它是「累計越限」而不是嚴格的連續濾波；若持續越限，最快約為 E1/E2 的 25.01 ms、E3 的 15.01 ms。

以上時間還受前台是否及時消費單槽 `Data_update_flag` 影響；`Delay_ms(1000)` 期間前台保護完全不掃描，因此不能把推算值當成已量測的故障關斷保證。

## 七個不同檔案的完整分類

| 相對路徑 | 差異類型 | 是否改變目標韌體行為 | 說明 |
|---|---|---|---|
| `mycode/state_machine.c` | 控制／保護設定 | 是 | `Target_voltage`、`Vout_ovp` |
| `mycode/PID.c` | 控制設定 | 是，部分差異實際無效 | PI 係數、輸出上限、`Pulse_width` 初始值與檔頭日期 |
| `mycode/HRTIM.c` | 即時排程／啟動安全 | 是 | REP=9/9/5；E1 額外執行 `DISdriver` |
| `mycode/PID.h` | 註解中繼資料 | 否 | E1 與 E2/E3 的檔頭日期不同 |
| `User_code/G4.uvoptx` | Keil 使用者／除錯選項 | 不屬於控制演算法 | Debug Driver 字串、Watch 變數等不同 |
| `User_code/G4.uvguix.Yatao` | Keil GUI 工作區 | 否 | 視窗、分頁、搜尋歷史與本機路徑不同 |
| `User_code/Listings/G4.map` | 建置產生檔 | 間接反映既有建置 | E1 映像摘要與 E2/E3 不同；不是參數來源 |

`G4.map` 的 E2 與 E3 逐位元組相同；E1 的既有 map 顯示 Total ROM 15,392 bytes，E2/E3 為 15,384 bytes。這只能描述倉庫中已提交的歷史建置產物，不代表目前環境重建後必然相同。

## 逐項原始碼分析

### 1. 目標電壓不是唯一需要調整的電壓設定

三套工程都在 `Reset_VAR()` 內重新載入運行參數：

- E1：[5 V 目標、10 V OVP](../例程1_左边12V输入右边5V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/mycode/state_machine.c#L61)
- E2：[12 V 目標、15 V OVP](../例程2_左边12V输入右边12V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/mycode/state_machine.c#L61)
- E3：[16 V 目標、19 V OVP](../例程3_左边12V输入右边16V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/mycode/state_machine.c#L61)

`Target_voltage` 不是直接寫入 DAC 或 PWM，而是作為軟啟動 `Vref` 的終點。ISR 先執行 `PID_loop(Vout)`，再以每次 0.001 V 增加 `Vref`：[E1 `state_machine.c:191`](../例程1_左边12V输入右边5V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/mycode/state_machine.c#L191)。

`Vout_ovp` 則由前台狀態機比較量測的 `Vout`，連續計數超過 200 後回到初始／關斷狀態：[E1 `state_machine.c:131`](../例程1_左边12V输入右边5V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/mycode/state_machine.c#L131)。三個 OVP 門檻相對目標值的裕量分別是 +5 V、+3 V、+3 V，並不是固定比例規則。因此不能從其中一個版本線性推導新門檻；門檻仍需依負載、容差、瞬態與元件額定值決定。

輸入保護與電流保護設定完全相同：`Vin_ovp = 24.0f`、`Vin_uvp = 8.0f`、`Iout_ocp = 3.6f`。ADC 工程量換算也相同，例如 `Vout` 都使用同一個 `0.011f` 係數：[E1 `state_machine.c:184`](../例程1_左边12V输入右边5V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/mycode/state_machine.c#L184)。這表示倉庫假定三個版本使用相同感測比例；靜態相同不等於已證明 16 V 版本具有足夠 ADC 頭房。

### 2. 例程 3 重新調整 PI，不只是提高參考值

PI 初始化位置：

- E1：[E1 `PID.c:23`](../例程1_左边12V输入右边5V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/mycode/PID.c#L23)，`Ki = 0.001`、`Kp = 0.01`。
- E2：[E2 `PID.c:23`](../例程2_左边12V输入右边12V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/mycode/PID.c#L23)，`Ki = 0.001`、`Kp = 0.01`。
- E3：[E3 `PID.c:23`](../例程3_左边12V输入右边16V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/mycode/PID.c#L23)，`Ki = 0.002`、`Kp = 0.015`。

三者採相同的增量式更新：

```text
error1 = Vref - Vout
error_add += (Ki + Kp) × error1 - Kp × error2
error2 = error1
```

實作位置見 [E1 `PID.c:32`](../例程1_左边12V输入右边5V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/mycode/PID.c#L32)。E3 同時提高單次更新係數並提高更新頻率，所以不能把 E1/E2 的離散時間響應直接外推到 E3。原始碼沒有補償設計、波德圖或調參紀錄；「因不同工作點而重新調參」是合理推測，不是已被倉庫證據證實的設計說明。

### 3. 最大占空比也按版本改變

三套工程都使用：

```text
PWM_K = PWM_PERIOD × 0.303030 ≈ PWM_PERIOD / 3.3
Pulse_width = PWM_K × V_PI.error_add
Duty ≈ V_PI.error_add / 3.3
```

`PWM_K` 定義見 [E1 `PID.c:16`](../例程1_左边12V输入右边5V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/mycode/PID.c#L16)。E1 把 `error_add` 上限設為 1.65，約等於 50% duty：[E1 `PID.c:38`](../例程1_左边12V输入右边5V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/mycode/PID.c#L38)。E2 與 E3 上限為 2.145，約等於 65% duty：[E2 `PID.c:38`](../例程2_左边12V输入右边12V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/mycode/PID.c#L38)、[E3 `PID.c:38`](../例程3_左边12V输入右边16V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/mycode/PID.c#L38)。

三者下限都是 0.05，約等於 1.515% duty。以 `PWM_PERIOD = 6800` 推算，運行時限幅約為：

| 工程 | 最小 CMP1 | 最大 CMP1 | 名義 duty 範圍 |
|---|---:|---:|---:|
| E1 | 約 103 count | 約 3,399 count | 約 1.515%～50% |
| E2 | 約 103 count | 約 4,419 count | 約 1.515%～65% |
| E3 | 約 103 count | 約 4,419 count | 約 1.515%～65% |

`Pulse_width` 最後寫入 CMP1，而 CMP3 設為脈寬一半，令 ADC 觸發點隨 duty 移動：[E1 `PID.c:41`](../例程1_左边12V输入右边5V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/mycode/PID.c#L41)。所以最大 duty 的改變也擴大 ADC 觸發相位範圍，不只是功率輸出能力的改變。

### 4. `PID_INT()` 的 0.01／0.015 係數差異目前不會套用到 PWM

`PID_INT()` 對 `Pulse_width` 的初值分別是：

- E1、E3：`0.015f * PWM_PERIOD * 0.303030f`，約 30 count，約為週期的 0.455%。
- E2：`0.01f * PWM_PERIOD * 0.303030f`，約 20 count，約為週期的 0.303%。

位置見 [E1 `PID.c:27`](../例程1_左边12V输入右边5V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/mycode/PID.c#L27)、[E2 `PID.c:27`](../例程2_左边12V输入右边12V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/mycode/PID.c#L27) 與 [E3 `PID.c:27`](../例程3_左边12V输入右边16V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/mycode/PID.c#L27)。

然而 `PID_INT()` 只修改 RAM 中的 `Pulse_width`，沒有寫 CMP1/CMP3。第一次 `PID_loop()` 又會先以 `PWM_K * error_add` 覆寫 `Pulse_width`，之後才寫硬體暫存器。全工程搜尋也找不到其他會使用這個初值的程式。因此這個 0.01／0.015 係數差異目前最多影響啟動等待期間的除錯觀察值，不能當成已生效的起始 PWM 設定。

真正會在輸出剛使能時存在的 HRTIM 初值是三者相同的 `CMP1 = 500`、`CMP3 = 1000`：[E1 `HRTIM.c:134`](../例程1_左边12V输入右边5V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/mycode/HRTIM.c#L134)。前台依序執行 `ENdriver`、使能 TA1/TA2，之後才把 `flag_start_cnt` 設為 `STOP` 允許 ISR 執行 PI：[E1 `state_machine.c:155`](../例程1_左边12V输入右边5V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/mycode/state_machine.c#L155)。這留下以共同初始 CMP 值短暫輸出的可能窗口，實際波形需板測確認。

### 5. 例程 3 改變 REP，連帶改變所有 tick-based 行為

三套工程的 `PWM_PERIOD` 都是 6800：[E1 `state_machine.h:23`](../例程1_左边12V输入右边5V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/mycode/state_machine.h#L23)。但 REP 設定不同：

- E1：[E1 `HRTIM.c:37`](../例程1_左边12V输入右边5V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/mycode/HRTIM.c#L37)，`RepetitionCounter = 0x09`。
- E2：[E2 `HRTIM.c:37`](../例程2_左边12V输入右边12V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/mycode/HRTIM.c#L37)，`RepetitionCounter = 0x09`。
- E3：[E3 `HRTIM.c:37`](../例程3_左边12V输入右边16V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/mycode/HRTIM.c#L37)，`RepetitionCounter = 0x05`。

REP ISR 同時負責設定 `Data_update_flag`、ADC 工程量換算、`PID_loop(Vout)` 與軟啟動遞增：[E1 `state_machine.c:176`](../例程1_左边12V输入右边5V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/mycode/state_machine.c#L176)。因此 REP 差異至少影響：

- 電壓 PI 每秒執行次數。
- `Vref += 0.001f` 的每秒累積速度。
- 前台取得新資料的最高通知頻率。
- 以 `OVP_CNT`／`OCP_CNT1` 表達的實際時間尺度。
- ISR CPU 負載與可用的最壞執行時間預算。

這是 E3 與 E1/E2 最重要的結構性差異之一。

### 6. E1 額外的 `DISdriver` 是安全時序漂移，不是合理的電壓調整

E1 在 `HRTIM_INT()` 設定初始 CMP 後立即執行 `DISdriver`：[E1 `HRTIM.c:138`](../例程1_左边12V输入右边5V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/mycode/HRTIM.c#L138)。E2 與 E3 對應位置沒有這行：[E2 `HRTIM.c:134`](../例程2_左边12V输入右边12V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/mycode/HRTIM.c#L134)、[E3 `HRTIM.c:134`](../例程3_左边12V输入右边16V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/mycode/HRTIM.c#L134)。

三套工程進入狀態機後，仍都會由 `Reset_VAR()` 執行 `DISdriver` 並以 `ODISR` 關閉 TA1/TA2：[E1 `state_machine.c:47`](../例程1_左边12V输入右边5V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/mycode/state_machine.c#L47)。差別在於 HRTIM 初始化完成至狀態機第一次 `Reset_VAR()` 之間，E2/E3 少了一次明確的 PA11 關斷寫入。

原始碼沒有證據顯示這是 5 V、12 V 或 16 V 所需的差異。較安全的分類是「版本漂移／待確認的啟動安全不一致」，不能在建立新電壓版本時照表複製。若要統一，應先確認 PA11 上電預設、外部 gate-driver enable 極性，以及 HRTIM 啟動期間 TA1/TA2 波形。

### 7. Keil 個人設定與 map 不是電壓控制參數

`G4.uvprojx` 在三套工程逐位元組相同，表示 target 名稱、STM32G474CETx 裝置、編譯器選擇、最佳化與巨集沒有因輸出電壓而分流。可由 [E1 `G4.uvprojx:10`](../例程1_左边12V输入右边5V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/User_code/G4.uvprojx#L10)、[裝置設定](../例程1_左边12V输入右边5V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/User_code/G4.uvprojx#L17) 與 [最佳化設定](../例程1_左边12V输入右边5V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/User_code/G4.uvprojx#L315) 定位。

不同的是 `G4.uvoptx` 與 `G4.uvguix.Yatao`。例如 E2 的 Watch 清單包含 `Current_State` 等項目：[E2 `G4.uvoptx:161`](../例程2_左边12V输入右边12V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/User_code/G4.uvoptx#L161)，E1/E3 則以 `Vin`、`Vout`、`Iin`、`Iout`、`IL_average` 開頭：[E1 `G4.uvoptx:161`](../例程1_左边12V输入右边5V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/User_code/G4.uvoptx#L161)。這些是 IDE 工作階段與除錯便利性差異，不應納入輸出電壓版本的功能設定。

`PID.h` 唯一差異是檔頭日期：E1 為 `12-Step-2022`，[E1 `PID.h:5`](../例程1_左边12V输入右边5V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/mycode/PID.h#L5)；E2/E3 為 `12-Jan-2023`，[E2 `PID.h:5`](../例程2_左边12V输入右边12V3A输出_PI电压单闭环_开关频率800KHz_具备过压欠压过流保护/mycode/PID.h#L5)。宣告與資料結構沒有功能差異。

## 從 E1 到 E2、從 E2 到 E3 實際改了什麼

### E1（5 V）→ E2（12 V）

會影響控制或保護的變更：

1. `Target_voltage`：5 V → 12 V。
2. `Vout_ovp`：10 V → 15 V。
3. PI 輸出上限：約 50% → 65% duty。
4. `PID_INT()` 的 `Pulse_width` RAM 初值：約 30 → 20 count（約 0.455% → 0.303% duty），但目前不會套用到 CMP。
5. 移除 `HRTIM_INT()` 內的 `DISdriver`；這是啟動安全差異，沒有證據能歸因於 12 V 設計。

保持不變：Kp/Ki、REP=9、800 kHz、ADC 換算、輸入 OVP/UVP、輸出 OCP、PI 下限、HRTIM 初始 CMP、Keil target 設定。

### E2（12 V）→ E3（16 V）

會影響控制或保護的變更：

1. `Target_voltage`：12 V → 16 V。
2. `Vout_ovp`：15 V → 19 V。
3. `Kp`：0.01 → 0.015；`Ki`：0.001 → 0.002。
4. REP：9 → 5，推算控制更新率 80 kHz → 約 133.3 kHz。
5. `PID_INT()` 的 `Pulse_width` RAM 初值：約 20 → 30 count（約 0.303% → 0.455% duty），但目前不會套用到 CMP。

保持不變：PI 上限約 65%、PI 下限約 1.515%、800 kHz、ADC 換算、輸入 OVP/UVP、輸出 OCP、HRTIM 初始 CMP、Keil target 設定。

## 哪些模組完全沒有因輸出電壓而修改

除前述 7 個檔案外，其餘 249 個檔案逐位元組相同。對應用層而言，以下模組沒有分流：

- `mycode/ADC.c/.h`：ADC channel、DMA、觸發與換算資料來源。
- `mycode/clock_config.c/.h`：系統時鐘與基礎 Timer 初始化。
- `mycode/Delay.c/.h`、`RGB.c/.h`、`Int.c/.h`。
- `mycode/common.h`、`Config.h`、`HRTIM.h`、`state_machine.h`。
- `User_code/main.c`、HAL MSP、中斷模板、system 檔與 linker scatter file。
- `User_code/G4.uvprojx`：MCU、編譯器、最佳化、defines 與 source group。
- 全部 `HAL_lib/` 與 `MDK/` 內容。

這說明三套版本確實共用同一控制骨架與周邊設定，但也代表任何共同缺陷會被完整複製。例如 ADC2 宣告尺寸不一致、單環 OCP 累計語意、1 秒忙等待與共享資料同步問題，在三套工程中都存在。

## 建立新輸出版本時的實務檢查表

若日後新增另一個輸出電壓，不建議直接照任一版本複製數字。至少應完成：

1. 依規格設定 `Target_voltage`，並確認 ADC 分壓與換算範圍能量測目標值及 OVP 值。
2. 依負載瞬態、量測誤差與元件額定值設定 `Vout_ovp`，不要只維持固定百分比或固定伏特裕量。
3. 由功率級拓撲與輸入範圍決定合法 duty 上下限；同時檢查 CMP1、死區與 CMP3 取樣位置。
4. 在已確定的 REP／取樣頻率下重新設計或驗證離散 PI，不把不同更新率的 Kp/Ki 直接互抄。
5. 以「V/s 或 ms」定義軟啟動，而不是只固定每 tick 增量；REP 改變時同步換算。
6. 以時間定義 OVP/OCP 濾波與鎖存需求，再換算成 tick；修正前要特別注意單環 OCP 是累計越限。
7. 統一並驗證啟動關斷順序，保留 PA11 `DISdriver` 與 HRTIM `ODISR` 的雙重門控。
8. 建置對應 Keil target，確認 warning 與 map；再以示波器量測開關頻率、REP、CMP3 取樣相位、啟動波形、穩態／瞬態響應與故障關斷延遲。

在完成上述量測前，本文只能回答「目前三套原始碼實際有哪些差異」，不能宣稱其中任何一組參數已對新硬體或新輸出電壓完成驗證。
