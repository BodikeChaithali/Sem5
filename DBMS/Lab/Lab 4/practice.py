import sqlite3

conn = sqlite3.connection("library.db");

cursor.execute(
	"INSERT INTO books values (?,?,?,?)",
	(1,"Database Systems","Korth",5)
)

#import psycopg #for postgresql 
#%s inplace of ? for postgres psql 
#sudo postgres -u psql

try:
	cursor.execute(
		"Update books set copies = copies - 1 where book_id = ?",
		(book_id,)
	)

	cursor.execute(
		"insert into loans (book_id,member_id,returned) values (?,?,0)",
		(book_id,member_id)
	)
	
	conn.commit()
except Exception:
	conn.rollback()

import psycopg

conn = psycopg.connect(
    "dbname=library user=postgres password=your_password"
)
