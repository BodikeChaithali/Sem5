-- Q1. Number of products and average price per category (show cat_name)
SELECT c.cat_name,
       COUNT(*) AS no_of_products,
       AVG(p.price) AS avg_price
FROM categories c
INNER JOIN products p
    ON c.cat_id = p.cat_id
GROUP BY c.cat_name;

-- Q2. Total revenue per category, highest first
SELECT c.cat_name,
       SUM(oi.quantity * oi.unit_price) AS revenue
FROM categories c
INNER JOIN products p
    ON c.cat_id = p.cat_id
INNER JOIN order_items oi
    ON p.prod_id = oi.prod_id
GROUP BY c.cat_name
ORDER BY revenue DESC;

-- Q3. Total revenue per customer country
SELECT c.country,
       SUM(oi.quantity * oi.unit_price) AS total_revenue
FROM customers c
INNER JOIN orders o
    ON c.cust_id = o.cust_id
INNER JOIN order_items oi
    ON oi.order_id = o.order_id
GROUP BY c.country;

-- Q4. Customers who placed more than 3 orders
SELECT c.name,
       COUNT(*) AS no_of_orders
FROM customers c
INNER JOIN orders o
    ON o.cust_id = c.cust_id
GROUP BY c.name
HAVING no_of_orders > 3;

-- Q5. For each product, total units sold, including products never sold
SELECT p.prod_name,
       COALESCE(SUM(oi.quantity), 0) AS total_units_sold
FROM products p
LEFT JOIN order_items oi
    ON p.prod_id = oi.prod_id
GROUP BY p.prod_name;

-- Q6. Months whose total revenue exceeded 15,000
SELECT strftime('%Y-%m', o.order_date) AS months,
       SUM(oi.quantity * oi.unit_price) AS total_revenue
FROM orders o
INNER JOIN order_items oi
    ON o.order_id = oi.order_id
GROUP BY strftime('%Y-%m', o.order_date)
HAVING total_revenue > 15000;

--Q7. For each category, number of products, average price, total units sold, and total revenue
SELECT c.cat_name,
       COUNT(DISTINCT p.prod_id) AS product_count,
       (SELECT AVG(p2.price)
        FROM products p2
        WHERE p2.cat_id = c.cat_id) AS avg_price,
       SUM(oi.quantity) AS units_sold,
       SUM(oi.quantity * oi.unit_price) AS revenue
FROM categories c
INNER JOIN products p
    ON p.cat_id = c.cat_id
INNER JOIN order_items oi
    ON oi.prod_id = p.prod_id
GROUP BY c.cat_id, c.cat_name;

-- Q8. Products priced above the overall average product price
SELECT prod_name, price
FROM products
WHERE price > (
    SELECT AVG(price)
    FROM products
);

-- Q9. Products priced above their own category's average price
SELECT p.price
FROM products p
WHERE p.price > (
    SELECT AVG(p1.price)
    FROM products p1
    WHERE p1.cat_id = p.cat_id
);

-- Q10. Customers who have never placed an order
-- NOT IN is risky because if the subquery returns a NULL,
-- the comparison becomes UNKNOWN and may cause valid customers
-- to be excluded. NOT EXISTS does not have this NULL problem.
SELECT *
FROM customers c
WHERE NOT EXISTS (
    SELECT *
    FROM orders o
    WHERE o.cust_id = c.cust_id
);

-- Q11. Customers who ordered in 2023 but not in 2022
SELECT *
FROM customers c
WHERE EXISTS (
    SELECT *
    FROM orders o1
    WHERE o1.cust_id = c.cust_id
      AND strftime('%Y', o1.order_date) = '2023'
)
AND NOT EXISTS (
    SELECT *
    FROM orders o2
    WHERE o2.cust_id = c.cust_id
      AND strftime('%Y', o2.order_date) = '2022'
);

-- Q12. The product(s) with the maximum price
SELECT *
FROM products p1
WHERE p1.price = (
    SELECT MAX(p2.price)
    FROM products p2
);

-- Q13. Customers who have bought every product in the Books category
SELECT c.*
FROM customers c
WHERE NOT EXISTS (
    SELECT *
    FROM products p
    WHERE p.cat_id = (
        SELECT cat_id
        FROM categories
        WHERE cat_name = 'Books'
    )
    AND NOT EXISTS (
        SELECT *
        FROM orders o
        INNER JOIN order_items oi
            ON o.order_id = oi.order_id
        WHERE o.cust_id = c.cust_id
          AND oi.prod_id = p.prod_id
    )
);
-- Q13 Answer:
-- Mia Schmidt qualifies because the query returned her as a customer
-- who has bought every product in the Books category.

-- Q14. The customer with the highest revenue in each country
WITH cust_revenue(cust_id, revenue) AS (
    SELECT o.cust_id,
           SUM(oi.quantity * oi.unit_price)
    FROM orders o
    INNER JOIN order_items oi
        ON o.order_id = oi.order_id
    GROUP BY o.cust_id
),
ranked_customers AS (
    SELECT c.name,
           c.country,
           cr.revenue,
           RANK() OVER (
               PARTITION BY c.country
               ORDER BY cr.revenue DESC
           ) AS rnk
    FROM customers c
    INNER JOIN cust_revenue cr
        ON c.cust_id = cr.cust_id
)
SELECT name,
       country,
       revenue
FROM ranked_customers
WHERE rnk = 1;

-- Q15. All transitive prerequisites of CS-347
WITH RECURSIVE prerequisites(course_id) AS (
    SELECT prereq_id
    FROM prereq
    WHERE course_id = 'CS-347'

    UNION

    SELECT p.prereq_id
    FROM prereq p
    INNER JOIN prerequisites pr
        ON p.course_id = pr.course_id
)
SELECT course_id
FROM prerequisites;