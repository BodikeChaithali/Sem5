# Lab 4 — Programming with a DBMS

## Part 2.1 — Transactions

The two writes in `borrow_book()` that must happen together are:

1. Decreasing the number of available copies in the `books` table.
2. Creating a new row in the `loans` table.

If only the number of copies is decreased but the loan is not recorded, the database loses track of who borrowed the book. On the other hand, if the loan is recorded but the number of copies is not decreased, the database may show more available copies than actually exist. Therefore, both operations must be completed as one transaction.

When the book has 0 copies, `borrow_book()` raises a `ValueError` with the message `no copies available`. The exception is caught by the `except` block, and `conn.rollback()` is executed there. This restores the database to its previous state.

## Part 2.2 — Atomicity Test

I attempted to borrow book 4 (`SQL Performance Explained`) using member 2.

The operation failed with:

`borrow failed (rolled back): no copies available`

Book 4 had 0 copies before the operation and still had 0 copies after the operation.

I also checked the outstanding loans list and no new loan for book 4 was created.

This shows that the failed transaction was rolled back and the database remained unchanged.

## Part 2.3 — Atomicity Test

I deliberately inserted `raise RuntimeError("boom")` between the UPDATE and INSERT statements in `borrow_book()`.

Before the failed transaction, the number of copies was: **2**

After the failed transaction, the number of copies was: **2**

The value remained unchanged because the exception caused `conn.rollback()` to execute. Therefore, the UPDATE was undone and the database was restored to its original state.

## Part 3.2 — Why the SQL Injection Works

The unsafe search function builds the SQL statement by concatenating the user's input directly into the query string. Because the input becomes part of the SQL command itself, an attacker can insert SQL syntax such as `OR`, comments, or `UNION` to change what the query does. The database therefore executes the attacker's input as SQL instead of treating it as ordinary search text.

## Part 4.3 — Changes from SQLite to PostgreSQL

The application required only a few changes to work with PostgreSQL:

1. The PostgreSQL connection string in `connect()` was configured with the correct host, database name, username and password.
2. SQLite uses `?` as the parameter placeholder, while PostgreSQL uses `%s`. The program already handles this through the `PH` variable.
3. The `loan_id` column was changed to an identity column so that PostgreSQL automatically generates loan IDs.
4. The same SQL operations and application logic were reused for both database systems.
5. No separate application was required; the same `app.py` runs with either backend.

