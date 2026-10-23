-- ============================================================
-- TASK 1
-- Monthly Account Cash Flow & Liquidity Ratio
-- GROUP BY, HAVING/Filtering, CASE
-- ============================================================

WITH monthly_flow AS (
    SELECT
        a.account_id,
        u.name AS user_name,
        strftime('%Y-%m', t.txn_date) AS txn_month,

        SUM(CASE
                WHEN t.status = 'success'
                 AND (t.txn_type = 'deposit' OR 
                        (t.txn_type = 'p2p' AND t.receiver_account_id = a.account_id)
                     )
                 THEN t.amount
                ELSE 0
            END
        ) AS total_inflow,

        SUM(CASE
                WHEN t.status = 'success'
                 AND (t.txn_type IN ('withdrawal', 'merchant_payment') OR 
                        (t.txn_type = 'p2p' AND t.sender_account_id = a.account_id)
                     )
                 THEN t.amount
                ELSE 0
            END
        ) AS total_outflow

    FROM accounts a
    JOIN users u
        ON u.user_id = a.user_id

    LEFT JOIN transactions t
        ON (
            t.sender_account_id = a.account_id
            OR t.receiver_account_id = a.account_id
        )
        AND t.status = 'success'

    GROUP BY
        a.account_id,
        u.name,
        strftime('%Y-%m', t.txn_date)
)

SELECT
    account_id,
    user_name,
    txn_month,
    ROUND(total_inflow, 2) AS total_inflow,
    ROUND(total_outflow, 2) AS total_outflow,
    ROUND(total_inflow - total_outflow, 2) AS net_cash_flow,

    CASE
        WHEN total_inflow - total_outflow > 0
            THEN 'Surplus'
        WHEN total_inflow - total_outflow < 0
            THEN 'Deficit'
        ELSE 'Balanced'
    END AS liquidity_status

FROM monthly_flow

WHERE total_inflow > 0
   OR total_outflow > 0

ORDER BY net_cash_flow DESC;


-- ============================================================
-- TASK 2
-- Merchant Category Leaders & Market Share
-- Window Functions + DENSE_RANK
-- ============================================================

WITH merchant_volumes AS (
    SELECT
        m.merchant_id,
        m.merchant_name,
        m.category,
        SUM(t.amount) AS merchant_volume

    FROM merchants m

    JOIN transactions t
        ON t.merchant_id = m.merchant_id

    WHERE t.txn_type = 'merchant_payment'
      AND t.status = 'success'

    GROUP BY
        m.merchant_id,
        m.merchant_name,
        m.category
),

ranked_merchants AS (

    SELECT
        merchant_id,
        merchant_name,
        category,
        merchant_volume,

        SUM(merchant_volume)
            OVER (PARTITION BY category)
            AS category_total_volume,

        DENSE_RANK()
            OVER (
                PARTITION BY category
                ORDER BY merchant_volume DESC
            )
            AS category_rank

    FROM merchant_volumes
)

SELECT
    merchant_id,
    merchant_name,
    category,
    ROUND(merchant_volume, 2) AS merchant_volume,
    ROUND(category_total_volume, 2) AS category_total_volume,

    ROUND(
        merchant_volume * 100.0
        / category_total_volume,
        2
    ) AS market_share_pct,

    category_rank

FROM ranked_merchants

WHERE category_rank <= 2

ORDER BY
    category ASC,
    category_rank ASC;


-- ============================================================
-- TASK 3
-- Referral Network Depth & Viral Reach
-- Recursive CTE
-- ============================================================

WITH RECURSIVE referral_tree AS (
    -- Seed users
    SELECT
        user_id,
        name,
        referrer_id,
        0 AS referral_depth,
        name AS referral_chain

    FROM users

    WHERE referrer_id IS NULL

    UNION ALL

    -- Recursive part
    SELECT
        u.user_id,
        u.name,
        u.referrer_id,
        rt.referral_depth + 1,

        rt.referral_chain
        || ' -> '
        || u.name

    FROM users u

    JOIN referral_tree rt
        ON u.referrer_id = rt.user_id
)

SELECT
    user_id,
    name,
    referral_depth,
    referral_chain

FROM referral_tree

WHERE referral_depth >= 1

ORDER BY
    referral_depth ASC,
    name ASC;


-- ============================================================
-- TASK 4
-- Suspicious Velocity & Rapid Multi-Transaction Spikes
-- LAG + Date/Time Difference
-- ============================================================

WITH p2p_lagged AS (

    SELECT
        txn_id,
        sender_account_id,
        txn_date,
        txn_time,
        amount,

        LAG(txn_time)
            OVER (
                PARTITION BY sender_account_id
                ORDER BY txn_date, txn_time
            )
            AS prev_txn_time,

        LAG(txn_date)
            OVER (
                PARTITION BY sender_account_id
                ORDER BY txn_date, txn_time
            )
            AS prev_txn_date,

        LAG(amount)
            OVER (
                PARTITION BY sender_account_id
                ORDER BY txn_date, txn_time
            )
            AS prev_amount

    FROM transactions

    WHERE txn_type = 'p2p'
),

velocity_check AS (

    SELECT
        *,
        CAST(
            (
                JULIANDAY(
                    txn_date || ' ' || txn_time
                )
                -
                JULIANDAY(
                    prev_txn_date || ' ' || prev_txn_time
                )
            ) * 86400
            AS INTEGER
        ) AS time_gap_seconds

    FROM p2p_lagged
)

SELECT
    txn_id,
    sender_account_id,
    txn_date,
    txn_time,
    amount,
    prev_txn_date,
    prev_txn_time,
    prev_amount,
    time_gap_seconds,

    CASE
        WHEN time_gap_seconds <= 300
         AND (amount + prev_amount) >= 2000.00
        THEN 'RAPID HIGH-VALUE VELOCITY'
        ELSE 'NORMAL'
    END AS velocity_risk_flag

FROM velocity_check

WHERE prev_txn_time IS NOT NULL

ORDER BY
    sender_account_id ASC,
    txn_date ASC,
    txn_time ASC;


-- ============================================================
-- TASK 5
-- High-Risk Account Audit View
-- CREATE VIEW + Correlated Subqueries + EXISTS
-- ============================================================

DROP VIEW IF EXISTS v_flagged_account_audit;

CREATE VIEW v_flagged_account_audit AS

SELECT
    a.account_id,
    u.name AS user_name,
    u.kyc_status,
    a.account_type,
    a.balance,

    -- Count failed or reversed transactions
    (
        SELECT COUNT(*)

        FROM transactions t

        WHERE (
            t.sender_account_id = a.account_id
            OR t.receiver_account_id = a.account_id
        )

        AND t.status IN ('failed', 'reversed')

    ) AS failed_or_reversed_txn_count,

    -- Maximum risk score
    (
        SELECT MAX(ra.risk_score)

        FROM risk_alerts ra

        JOIN transactions t2
            ON t2.txn_id = ra.txn_id

        WHERE (
            t2.sender_account_id = a.account_id
            OR t2.receiver_account_id = a.account_id
        )

    ) AS max_risk_score

FROM accounts a

JOIN users u
    ON u.user_id = a.user_id

WHERE u.kyc_status != 'verified'

AND (
    -- Failed or reversed transaction exists
    EXISTS (
        SELECT 1

        FROM transactions t

        WHERE (
            t.sender_account_id = a.account_id
            OR t.receiver_account_id = a.account_id
        )

        AND t.status IN ('failed', 'reversed')
    )

    OR

    -- High-risk alert exists
    EXISTS (
        SELECT 1

        FROM risk_alerts ra

        JOIN transactions t2
            ON t2.txn_id = ra.txn_id

        WHERE (
            t2.sender_account_id = a.account_id
            OR t2.receiver_account_id = a.account_id
        )

        AND ra.risk_score >= 80
    )
);


-- Verify the view
SELECT *
FROM v_flagged_account_audit
ORDER BY balance DESC;