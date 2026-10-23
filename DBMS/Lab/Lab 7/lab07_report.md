# Lab 07 — Advanced SQL Synthesis & Analytics

## FinPay Digital Wallet and Risk Engine

---

# Task 1 — Monthly Account Cash Flow & Liquidity Ratio

### Output

| account_id | user_name | txn_month | total_inflow | total_outflow | net_cash_flow | liquidity_status |
|---:|---|---|---:|---:|---:|---|
| 101 | Aarav Sharma | 2026-01 | 10000.0 | 5450.0 | 4550.0 | Surplus |
| 107 | Yuki Tanaka | 2026-01 | 3000.0 | 0.0 | 3000.0 | Surplus |
| 104 | Sophia Martinez | 2026-01 | 1300.0 | 0.0 | 1300.0 | Surplus |
| 103 | Liam Chen | 2026-01 | 6200.0 | 5000.0 | 1200.0 | Surplus |
| 106 | Carlos Gomez | 2026-01 | 0.0 | 7700.0 | -7700.0 | Deficit |

### Explanation

The query joins accounts, users, and successful transactions and uses conditional `SUM(CASE...)` expressions to calculate incoming and outgoing cash flows. The net cash flow is calculated as inflow minus outflow, and a `CASE` expression classifies each account as Surplus, Deficit, or Balanced.

---

# Task 2 — Merchant Category Leaders & Market Share

### Output

| merchant_id | merchant_name | category | merchant_volume | category_total_volume | market_share_pct | category_rank |
|---:|---|---|---:|---:|---:|---:|
| 205 | Bistro Deluxe | Food | 450.0 | 450.0 | 100.0 | 1 |
| 201 | MegaMart Retail | Retail | 4300.0 | 4300.0 | 100.0 | 1 |
| 203 | SkyWings Airlines | Travel | 6500.0 | 9700.0 | 67.01 | 1 |
| 204 | Globe Trotter Travels | Travel | 3200.0 | 9700.0 | 32.99 | 2 |
| 206 | PowerGrid Utilities | Utilities | 1200.0 | 1200.0 | 100.0 | 1 |

### Explanation

The query first aggregates successful merchant-payment volume for each merchant. Window functions calculate the total volume within each category and use `DENSE_RANK()` to rank merchants, after which only the top two merchants per category are returned.

---

# Task 3 — Referral Network Depth & Viral Reach

### Output

| user_id | name | referral_depth | referral_chain |
|---:|---|---:|---|
| 5 | Carlos Gomez | 1 | Aarav Sharma -> Carlos Gomez |
| 2 | Liam Chen | 1 | Aarav Sharma -> Liam Chen |
| 3 | Sophia Martinez | 2 | Aarav Sharma -> Liam Chen -> Sophia Martinez |
| 6 | Yuki Tanaka | 2 | Aarav Sharma -> Carlos Gomez -> Yuki Tanaka |
| 4 | Emma Watson | 3 | Aarav Sharma -> Liam Chen -> Sophia Martinez -> Emma Watson |

### Explanation

A recursive CTE starts with users who have no referrer and assigns them depth 0. Each recursive step finds users referred by the previous level, increments the depth, and builds the complete referral path using string concatenation.

---

# Task 4 — Suspicious Velocity & Rapid Multi-Transaction Spikes

### Output

| txn_id | sender_account_id | txn_date | txn_time | amount | prev_txn_date | prev_txn_time | prev_amount | time_gap_seconds | velocity_risk_flag |
|---:|---:|---|---|---:|---|---|---:|---:|---|
| 5 | 101 | 2026-01-10 | 14:02:30 | 1300 | 2026-01-10 | 14:00:00 | 1200 | 149 | RAPID HIGH-VALUE VELOCITY |
| 13 | 105 | 2026-02-01 | 10:18:00 | 2000 | 2026-02-01 | 10:15:00 | 1500 | 179 | RAPID HIGH-VALUE VELOCITY |

### Explanation

The query uses `LAG()` to retrieve the previous P2P transaction for each sender and calculates the time gap between consecutive transactions. Transactions occurring within five minutes with a combined value of at least 2000 are classified as RAPID HIGH-VALUE VELOCITY.

---

# Task 5 — High-Risk Account Audit View

### Output

| account_id | user_name | kyc_status | account_type | balance | failed_or_reversed_txn_count | max_risk_score |
|---:|---|---|---|---:|---:|---:|
| 108 | Noah Smith | flagged | wallet | 5400 | 1 | 95 |
| 107 | Yuki Tanaka | pending | checking | 2100 | 1 | 85 |
| 104 | Sophia Martinez | pending | wallet | 850 | 1 | 88 |
| 105 | Emma Watson | flagged | wallet | 150 | 2 | 92 |

### Explanation

The view selects accounts belonging to users whose KYC status is not verified and checks for either failed/reversed transactions or risk alerts with scores of at least 80. Correlated subqueries calculate the number of failed/reversed transactions and the maximum risk score associated with each account.

---
