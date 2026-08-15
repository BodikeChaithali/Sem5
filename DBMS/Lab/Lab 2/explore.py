import sqlite3
import sys

conn = sqlite3.connect("lab2.db")
conn.row_factory = sqlite3.Row

cur = conn.cursor()

# Get department name from the command line
dept = sys.argv[1]

# Parameterized query
cur.execute(
    "SELECT name, salary FROM instructor WHERE dept_name = ?",
    (dept,)
)

# Print all instructors in the department
for row in cur.fetchall():
    print(row["name"], row["salary"])

# Aggregate
cur.execute("SELECT COUNT(*) AS n FROM student")
print("total students:", cur.fetchone()["n"])

conn.close()
