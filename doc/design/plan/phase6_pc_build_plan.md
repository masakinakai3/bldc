# Phase 6 — 統合テスト実行計画（PCビルド）

作成日: 2025-12-26

## 目的
- `pc_build` に移植したVESCロジックを統合検証し、シミュレーション／テスト資産の信頼性を確保する。

## 範囲
- モーター制御（FOC）統合シミュレーション、データ記録、回帰テスト、CI連携までを対象とする。
- ターゲット環境: WSL (Ubuntu)、開発者PC上の GCC ビルド

## 前提条件
- `make phase5` と `make test_phase5` がローカルで成功すること（`pc_build` が構成済み）。
- 参考資料: `doc/design/pc_build_strategy.md`（PC用ビルド戦略）

## 成果物
- テスト実行ログ（`pc_build/build/` と `pc_build/results/` の出力CSV）
- 回帰テスト合格レポート（`doc/design/test/phase6_verification_report.md`）
- CI ジョブ定義（例: `.github/workflows/pc_build_phase6.yml`、オプション）

## タイムライン（1週間想定）
- Day 0: 準備 — 前提確認、WSL環境整備、依存ツール確認（`make,gcc,git`）。
- Day 1: 統合テストケース整理 — 既存の `pc_build/tests/*` をレビューし必須ケースを確定。
- Day 2: 自動実行スクリプト作成 — `pc_build/run_phase6.sh`（または Make ターゲット）で一括実行。
- Day 3: リファレンス生成 & ベースライン測定 — 代表シナリオ（速度ステップ・トルクステップ・HFI）を実行して基準データを取得。
- Day 4: 回帰テスト実行 — `make test_phase5` を実行し失敗ケースを修正/許容値調整。
- Day 5: CI 統合準備 — GitHub Actions / Jenkins 用ジョブ定義を作成。
- Day 6: ドキュメント作成 & レポート提出 — `doc/design/test/phase6_verification_report.md` を作成。

## 詳細作業項目（チェックリスト）
1. 環境確認
   - WSL が動作しているか確認: `wsl --status`
   - WSL 内で `gcc`, `make` が利用可能か確認
2. テスト実行スクリプト
   - `pc_build/run_phase6.sh` を作成（以下を実行）:
     ```bash
     cd pc_build
     make phase5
     make test_phase5
     ```
3. ベースライン生成
   - `pc_build/tests` の主要シナリオを実行し、`pc_build/reference/` と `pc_build/results/` に出力を保存
4. 回帰テスト
   - `pc_build/tests/test_regression` を実行し、閾値や統計処理により合否判定
5. CI ジョブ（推奨）
   - GitHub Actions サンプル:
     - ワークフロー: Ubuntu latest、WSL 不要（ネイティブ Ubuntu 環境）
     - 手順: checkout → apt-get install build-essential make → run `make test_phase5`
6. レポート作成
   - テストサマリ（合格/失敗、最大差分、平均差分）
   - 変更したテスト許容値・理由の一覧

## 受入基準
- 全てのユニットテストと統合テストが成功
- 回帰テストは基準内（事前合意の許容値）
- CI 上で再現可能な手順が用意されていること

## リスクと緩和策
- モデルと実機の差異により絶対一致しない点が多数ある
  - 緩和: 許容値を明示化し、差異の原因（モデル・ゲイン）をドキュメント化
- タイミング依存テストの不安定性
  - 緩和: シミュレーション時間・サンプル間隔を固定化、ランダムシードを導入

## コマンド例（実行手順）
```bash
# WSL 上での一括実行
cd /mnt/c/Users/galax/bldc/bldc/pc_build
make phase5
make test_phase5
```

## エスカレーション条件
- テストが CI 上で再現できない不一致を示した場合、原因調査（モデル差分／ゲイン／テスト欠陥）をトリアージして優先度高で修正する。

---

配置先: `doc/design/plan/phase6_pc_build_plan.md`
