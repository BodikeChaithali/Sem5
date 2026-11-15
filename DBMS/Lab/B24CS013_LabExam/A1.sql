SELECT
    I.id,
    COUNT(DISTINCT T.id) AS student_count
FROM Instructor I
LEFT JOIN Teaches T
    ON I.id = T.id
LEFT JOIN Takes S
    ON T.course_id = S.course_id
    AND T.sec_id = S.sec_id
    AND T.semester = S.semester
    AND T.year = S.year
GROUP BY I.id
HAVING COUNT(DISTINCT S.id) > (
    SELECT AVG(student_count)
    FROM (
        SELECT
            I.id,
            COUNT(DISTINCT S.id) AS student_count
        FROM Instructor I
        LEFT JOIN Teaches T ON I.id = T.id
        LEFT JOIN Takes S
            ON T.course_id = S.course_id
            AND T.sec_id = S.sec_id
            AND T.semester = S.semester
            AND T.year = S.year
        GROUP BY I.id
    )
);

-- Output
-- 10101|1
-- 22222|1
-- 76766|1
-- 83821|1