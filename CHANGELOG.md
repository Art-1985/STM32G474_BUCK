# 變更紀錄

格式參考 Keep a Changelog；目前尚未建立正式版本標籤。

## Unreleased

### Added

- 新增 `docs/C4.md`：System Context、單一韌體映像內的邏輯 Container、應用 Component、Deployment View、跨層關鍵路徑與架構風險。
- 新增 `docs/UML.md`：六個例程差異矩陣、模組 UML、前台狀態機、上電/閉環時序、控制資料流、並發資料所有權與程式碼審閱注意事項。
- 新增 `docs/CALL_GRAPH.md`：總呼叫圖、初始化呼叫樹、前台狀態機呼叫圖、HRTIM ISR/PI 分支、函式索引及硬體隱式控制邊。
- 新增 `ARCHITECTURE.md` 作為根目錄架構摘要與文件導覽。
- 新增 `AGENTS.md`、`PLAN.md`、`PROGRESS.md`、`DECISIONS.md`、`TASKS.md`、`MEMORY.md`，建立協作、計畫、追蹤、決策與長期記憶基線。

### Analysis

- 確認例程 1～3 為 800 kHz 電壓單環，例程 4～6 為 200 kHz 恆壓/恆流雙 PI。
- 記錄 HRTIM Timer A、ADC/DMA、REP ISR、PI 與前台保護之間的執行關係。
- 識別共享變數同步、ADC2 陣列宣告、OCP 計數語意、忙等待軟啟動、ADC 連續模式及硬體 fault 停用等待辦風險。

### Validation

- 使用 Mermaid CLI 實際渲染 `docs/` 中 11 個 Mermaid 圖表；連同根目錄 `ARCHITECTURE.md` 共 14 個。
- 檢查 Markdown 相對連結、程式碼圍欄與尾端空白。
- 尚未執行 Keil 韌體建置或硬體板測。
