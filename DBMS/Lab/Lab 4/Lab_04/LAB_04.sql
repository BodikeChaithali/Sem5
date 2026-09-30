-- 15
WITH cte_table AS(
SELECT *,RANK() OVER(PARTITION BY cat_id ORDER BY price DESC) AS rn FROM products)
SELECT * FROM cte_table WHERE rn<=2;

-- 16
WITH monthly_orders AS (
    SELECT
        strftime('%Y-%m', order_date) AS month,
        COUNT(*) AS order_count
    FROM orders
    GROUP BY strftime('%Y-%m', order_date)
)
SELECT
    month,
    order_count,
    SUM(order_count) OVER (ORDER BY month) AS running_total
FROM monthly_orders
ORDER BY month;

-- 17 

WITH customer_spend AS (
    SELECT
        c.cust_id,
        c.name,
        COALESCE(SUM(oi.quantity * oi.unit_price), 0) AS total_spend
    FROM customers c
    LEFT JOIN orders o ON o.cust_id = c.cust_id
    LEFT JOIN order_items oi ON oi.order_id = o.order_id
    GROUP BY c.cust_id, c.name
)
SELECT
    cust_id,
    name,
    total_spend,
    RANK() OVER (ORDER BY total_spend DESC) AS spend_rank,
    ROUND(
        100.0 * total_spend / MAX(total_spend) OVER (),
        2
    ) AS percentage_of_top_spender
FROM customer_spend
ORDER BY spend_rank;
-- 19
-- 
CREATE VIEW product_sales AS
SELECT p.prod_id,p.prod_name,
coalesce(SUM(oi.quantity),0) AS units,
coalesce(SUM(oi.quantity*oi.unit_price),0) AS revenue
FROM products p
LEFT JOIN order_items oi ON oi.prod_id=p.prod_id
GROUP BY p.prod_id,p.prod_name;

-- 20

SELECT * FROM product_sales ORDER BY revenue DESC LIMIT 5;
-- 21

SELECT * FROM product_sales WHERE 
units = 0;

-- Window functions are calculated after the WHERE clause. Therefore, we cannot directly use the window-function result rn in the WHERE clause of the same query.
