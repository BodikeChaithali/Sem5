WITH completed_orders AS (
    SELECT
        o.order_id,
        o.cust_id,
        o.order_date,
        SUM(oi.quantity * oi.unit_price) AS order_amount
    FROM orders o
    JOIN order_items oi ON o.order_id = oi.order_id
    WHERE o.status = 'completed'
    GROUP BY o.order_id, o.cust_id, o.order_date
),

order_gaps AS (
    SELECT
        cust_id,
        order_date,
        order_amount,
        julianday(order_date) - julianday(
            LAG(order_date) OVER (
                PARTITION BY cust_id
                ORDER BY order_date
            )
        ) AS gap_days
    FROM completed_orders
),

customer_stats AS (
    SELECT
        cust_id,
        COUNT(*) AS order_count,
        MAX(gap_days) AS longest_gap,
        SUM(order_amount) AS total_spend,
        MAX(order_amount) AS highest_order
    FROM order_gaps
    GROUP BY cust_id
)

SELECT
    c.cust_id,
    c.name,
    COALESCE(cs.order_count, 0) AS order_count,
    cs.longest_gap,
    ROUND(
        100.0 * cs.highest_order / NULLIF(cs.total_spend, 0),
        2
    ) AS highest_order_percentage
FROM customers c
LEFT JOIN customer_stats cs
    ON c.cust_id = cs.cust_id
ORDER BY
    cs.longest_gap DESC NULLS LAST;

-- Output
-- 18|Mia Schmidt|2|513.0|72.8
-- 17|Lucas Muller|4|467.0|41.39
-- 19|Hana Sato|3|386.0|53.66
-- 16|Sophia Garcia|2|382.0|54.42
-- 15|Mason Lee|2|311.0|80.01
-- 1|Aarav Sharma|2|291.0|71.41
-- 2|Diya Patel|3|290.0|59.71
-- 10|Emma Brown|2|273.0|61.79
-- 9|John Smith|3|217.0|62.13
-- 13|Noah Johnson|3|204.0|78.8
-- 14|Ava Martinez|3|204.0|45.51
-- 7|Reyansh Gupta|2|159.0|98.68
-- 12|Olivia Taylor|5|133.0|56.34
-- 5|Kabir Singh|2|70.0|68.03
-- 6|Isha Nair|2|1.0|94.47
-- 3|Vivaan Rao|1||100.0
-- 4|Ananya Iyer|1||100.0
-- 8|Myra Das|1||100.0
-- 11|Liam Wilson|0||
-- 20|Ren Tanaka|0||