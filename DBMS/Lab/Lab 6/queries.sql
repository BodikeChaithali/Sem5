-- =========================================================
-- TASK 1
-- User Viewing Profile & Binge Engagement
-- Aggregations, GROUP BY, HAVING, CASE
-- =========================================================

SELECT
    u.user_id,
    u.name,
    u.subscription_tier,
    SUM(w.watch_time_minutes) AS total_watch_time_mins,
    COUNT(DISTINCT w.content_id) AS titles_watched_count,
    ROUND(AVG(w.watch_time_minutes), 1) AS avg_session_duration,
    CASE
        WHEN SUM(w.watch_time_minutes) >= 350
            THEN 'Power Viewer'
        WHEN SUM(w.watch_time_minutes) >= 150
            THEN 'Regular Viewer'
        ELSE 'Light Viewer'
    END AS engagement_category
FROM users u
JOIN watch_history w
    ON u.user_id = w.user_id
GROUP BY
    u.user_id,
    u.name,
    u.subscription_tier
HAVING COUNT(DISTINCT w.content_id) >= 2
ORDER BY total_watch_time_mins DESC;


-- =========================================================
-- TASK 2
-- Top Genre Rankings per Subscription Tier
-- CTE + DENSE_RANK()
-- =========================================================

WITH genre_totals AS (
    SELECT
        u.subscription_tier,
        c.genre,
        SUM(w.watch_time_minutes) AS total_watch_time
    FROM users u
    JOIN watch_history w
        ON u.user_id = w.user_id
    JOIN content c
        ON w.content_id = c.content_id
    GROUP BY
        u.subscription_tier,
        c.genre
),
ranked_genres AS (
    SELECT
        subscription_tier,
        genre,
        total_watch_time,
        DENSE_RANK() OVER (
            PARTITION BY subscription_tier
            ORDER BY total_watch_time DESC
        ) AS genre_rank
    FROM genre_totals
)
SELECT
    subscription_tier,
    genre,
    total_watch_time,
    genre_rank
FROM ranked_genres
WHERE genre_rank <= 2
ORDER BY
    subscription_tier ASC,
    genre_rank ASC;


-- =========================================================
-- TASK 3
-- Inactive Active Subscriber Identification
-- Correlated NOT EXISTS Subquery
-- =========================================================

SELECT
    u.user_id,
    u.name,
    u.email,
    u.subscription_tier,
    MAX(w.watch_date) AS last_watch_date,
    ROUND(AVG(r.score), 2) AS avg_rating_given
FROM users u
JOIN subscriptions s
    ON u.user_id = s.user_id
LEFT JOIN watch_history w
    ON u.user_id = w.user_id
JOIN ratings r
    ON u.user_id = r.user_id
WHERE s.payment_status = 'active'
  AND NOT EXISTS (
        SELECT 1
        FROM watch_history w2
        WHERE w2.user_id = u.user_id
          AND w2.watch_date >= '2026-01-30'
          AND w2.watch_date < '2026-03-01'
  )
GROUP BY
    u.user_id,
    u.name,
    u.email,
    u.subscription_tier
HAVING AVG(r.score) >= 4.0
ORDER BY last_watch_date ASC;


-- =========================================================
-- TASK 4
-- User Session Continuity & Gap Analysis
-- LEAD() + JULIANDAY()
-- =========================================================

WITH sessions AS (
    SELECT
        w.watch_id,
        w.user_id,
        u.name,
        w.watch_date,
        LEAD(w.watch_date) OVER (
            PARTITION BY w.user_id
            ORDER BY w.watch_date
        ) AS next_watch_date
    FROM watch_history w
    JOIN users u
        ON w.user_id = u.user_id
)
SELECT
    watch_id,
    user_id,
    name,
    watch_date,
    next_watch_date,
    CAST(
        julianday(next_watch_date) - julianday(watch_date)
        AS INTEGER
    ) AS days_until_next_session,
    CASE
        WHEN julianday(next_watch_date) - julianday(watch_date) >= 14
            THEN 'LONG INACTIVITY BREAK'
        ELSE 'NORMAL'
    END AS session_gap_flag
FROM sessions
WHERE next_watch_date IS NOT NULL
ORDER BY
    user_id ASC,
    watch_date ASC;


-- =========================================================
-- TASK 5
-- Content Catalog Performance & View Creation
-- CREATE VIEW + LEFT JOIN
-- =========================================================

DROP VIEW IF EXISTS v_content_analytics;

CREATE VIEW v_content_analytics AS

WITH watch_stats AS (
    SELECT
        content_id,
        COUNT(*) AS total_views,
        SUM(watch_time_minutes) AS total_watch_minutes
    FROM watch_history
    GROUP BY content_id
),
rating_stats AS (
    SELECT
        content_id,
        ROUND(AVG(score), 2) AS avg_user_rating,
        COUNT(*) AS rating_count
    FROM ratings
    GROUP BY content_id
)

SELECT
    c.content_id,
    c.title,
    c.genre,
    c.release_year,
    c.type,
    COALESCE(w.total_views, 0) AS total_views,
    ROUND(
        COALESCE(w.total_watch_minutes, 0) / 60.0,
        2
    ) AS total_watch_hours,
    r.avg_user_rating,
    COALESCE(r.rating_count, 0) AS rating_count
FROM content c
LEFT JOIN watch_stats w
    ON c.content_id = w.content_id
LEFT JOIN rating_stats r
    ON c.content_id = r.content_id
WHERE c.release_year >= 2024;


-- Verify the created view
SELECT *
FROM v_content_analytics
ORDER BY total_watch_hours DESC;