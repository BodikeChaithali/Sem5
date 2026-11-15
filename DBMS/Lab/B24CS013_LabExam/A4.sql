WITH user_genre_minutes AS (
    SELECT
        c.genre,
        wh.user_id,
        SUM(wh.watch_time_minutes) AS total_minutes
    FROM watch_history wh
    JOIN content c
        ON wh.content_id = c.content_id
    GROUP BY c.genre, wh.user_id
),

genre_totals AS (
    SELECT
        genre,
        SUM(total_minutes) AS genre_minutes
    FROM user_genre_minutes
    GROUP BY genre
),

ranked_users AS (
    SELECT
        genre,
        user_id,
        total_minutes,
        RANK() OVER (
            PARTITION BY genre
            ORDER BY total_minutes DESC
        ) AS rnk
    FROM user_genre_minutes
)

SELECT
    ru.genre,
    u.user_id,
    u.name,
    ru.total_minutes,
    ROUND(
        100.0 * ru.total_minutes / gt.genre_minutes,
        2
    ) AS share_percentage,
    CASE
        WHEN EXISTS (
            SELECT 1
            FROM ratings r
            JOIN content c ON c.content_id = r.content_id
            WHERE r.user_id = ru.user_id
              AND c.genre = ru.genre
        )
        THEN 'yes'
        ELSE 'no'
    END AS has_rated
FROM ranked_users ru
JOIN genre_totals gt
    ON gt.genre = ru.genre
JOIN users u
    ON u.user_id = ru.user_id
WHERE ru.rnk = 1
ORDER BY ru.genre;

-- Output
-- Action|1|Aarav Sharma|130|50.0|yes
-- Action|3|Liam Chen|130|50.0|yes
-- Comedy|4|Emma Watson|85|50.0|yes
-- Comedy|5|Carlos Gomez|85|50.0|no
-- Documentary|1|Aarav Sharma|95|38.0|no
-- Documentary|6|Yuki Tanaka|95|38.0|yes
-- Drama|5|Carlos Gomez|155|43.66|yes
-- Sci-Fi|1|Aarav Sharma|190|33.33|yes
-- Sci-Fi|3|Liam Chen|190|33.33|yes