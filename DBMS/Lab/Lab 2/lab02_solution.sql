-- Q1:
SELECT title
FROM course
WHERE dept_name = 'Comp. Sci.';


-- Q2:
select
    name ,
    round(salary / 12.0,2) as monthly_pay
from instructor
where
    salary between 70000 and 90000
order by monthly_pay desc;


-- Q3:
select distinct semester
from section;


-- Q4:
select
    id,
    name,
    tot_cred
from student
order by tot_cred desc , name
limit 3;


-- Q5:
select
    course_id,
    title
from course
where (
    title like '%System%' or
    title like '%system%'
) and (
    not title like '%Intro%'
);


-- Q6:
select
    name,
    dept_name
from instructor
where dept_name not in ('History','Music')
order by dept_name,name;


-- Q7:
select
    count(*)
from instructor;


-- Q8:
SELECT name,
       salary,
       CASE
           WHEN salary >= 90000 THEN 'High'
           WHEN salary BETWEEN 70000 AND 89999 THEN 'Medium'
           ELSE 'Low'
       END AS salary_category
FROM instructor;


-- Q9:
select
    name,
    tot_cred
from student
where tot_cred % 10 = 0 and tot_cred != 0;


-- Q10:
select
    s.name ,
    d.building
from student s
inner join department d
    on d.dept_name = s.dept_name;


-- Q11:
select
    i.name,
    c.title
from instructor i
inner join teaches t
    on i.id = t.id
inner join section s
    on t.course_id = s.course_id
    and t.sec_id = s.sec_id
    and t.semester = s.semester
    and t.year = s.year
inner join course c
    on c.course_id = s.course_id;


-- Q12:
select
    st.name,
    c.title,
    t.grade
from student st
inner join takes t
    on st.id = t.id
inner join section s
    on t.course_id = s.course_id
    and t.sec_id = s.sec_id
    and t.semester = s.semester
    and t.year = s.year
inner join course c
    on c.course_id = s.course_id;


-- Q13:
SELECT s.name
FROM student s
INNER JOIN advisor a
    ON s.id = a.s_id
INNER JOIN instructor i
    ON a.i_id = i.id
WHERE i.name = 'Katz';


-- Q14:
SELECT c.title, cl.capacity
FROM section s
INNER JOIN classroom cl
    ON s.building = cl.building
    AND s.room_no = cl.room_no
INNER JOIN course c
    ON s.course_id = c.course_id;


-- Q15:
SELECT DISTINCT d.building
FROM section s
INNER JOIN course c
    ON s.course_id = c.course_id
INNER JOIN department d
    ON c.dept_name = d.dept_name;


-- Q16:
SELECT DISTINCT i.name
FROM instructor i
INNER JOIN teaches t
    ON i.id = t.id
INNER JOIN section s
    ON t.course_id = s.course_id
    AND t.sec_id = s.sec_id
    AND t.semester = s.semester
    AND t.year = s.year
WHERE s.building = 'Taylor';


-- Q17:
-- LEFT JOIN keeps every section, including sections with no instructor.
-- If a teaches column were filtered in WHERE, rows with no matching
-- instructor would have NULL values and would be removed, effectively
-- turning the LEFT JOIN into an INNER JOIN.

SELECT s.course_id,
       s.sec_id,
       s.semester,
       s.year,
       t.id AS instructor_id
FROM section s
LEFT JOIN teaches t
    ON s.course_id = t.course_id
    AND s.sec_id = t.sec_id
    AND s.semester = t.semester
    AND s.year = t.year;


-- Q18:
-- The WHERE t.id IS NULL condition keeps only sections that have
-- no matching row in teaches.

SELECT s.course_id,
       s.sec_id,
       s.semester,
       s.year
FROM section s
LEFT JOIN teaches t
    ON s.course_id = t.course_id
    AND s.sec_id = t.sec_id
    AND s.semester = t.semester
    AND s.year = t.year
WHERE t.id IS NULL;


-- Q19:
SELECT d.dept_name,
       COUNT(s.id) AS student_count
FROM department d
LEFT JOIN student s
    ON d.dept_name = s.dept_name
GROUP BY d.dept_name;


-- Q20:
SELECT i.id,
       i.name
FROM instructor i
LEFT JOIN teaches t
    ON i.id = t.id
WHERE t.id IS NULL;


-- Q21:
SELECT c.title,
       i.name AS instructor_name
FROM course c
LEFT JOIN teaches t
    ON c.course_id = t.course_id
LEFT JOIN instructor i
    ON t.id = i.id;


-- Q22:
-- All pairs of instructors in the same department,
-- with no self-pairs and no mirror duplicates.
SELECT a.name AS instructor1,
       b.name AS instructor2,
       a.dept_name
FROM instructor a
INNER JOIN instructor b
    ON a.dept_name = b.dept_name
    AND a.id < b.id;


-- Q23:
-- Each course title with the title of its prerequisite.
SELECT c.title AS course_title,
       p.title AS prerequisite_title
FROM prereq r
INNER JOIN course c
    ON r.course_id = c.course_id
INNER JOIN course p
    ON r.prereq_id = p.course_id;


-- Q24:
-- Row-explosion demo:
-- Every student is paired with every department because there is
-- no JOIN condition. Therefore the count is #students × #departments.
-- 15 students × 7 departments = 105 rows.
-- An ON clause supplies the matching condition needed to turn this
-- into a meaningful join.

SELECT COUNT(*)
FROM student, department;


-- Q25:
-- INNER JOIN returns only sections that have a matching instructor.
-- LEFT JOIN keeps all sections, including sections with no instructor.

SELECT
    (SELECT COUNT(*)
     FROM section s
     INNER JOIN teaches t
         ON s.course_id = t.course_id
         AND s.sec_id = t.sec_id
         AND s.semester = t.semester
         AND s.year = t.year) AS inner_join_count,

    (SELECT COUNT(*)
     FROM section s
     LEFT JOIN teaches t
         ON s.course_id = t.course_id
         AND s.sec_id = t.sec_id
         AND s.semester = t.semester
         AND s.year = t.year) AS left_join_count;

/*
Part 5 explore.py output:

Einstein 95000
Gold 87000
total students: 15
*/