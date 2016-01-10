#ifndef DATABASESQL_HPP
#define DATABASESQL_HPP

#include <array>

int const DatabaseVersion = 1;

std::array<std::string const, DatabaseVersion> const SqlMigrationSteps
{
    R"(
PRAGMA user_version = 1;

PRAGMA foreign_keys = ON;

CREATE TABLE Device (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL
);

CREATE TABLE KeyEvent (
    id INTEGER PRIMARY KEY,
    device INTEGER NOT NULL,
    timestamp INTEGER NOT NULL,
    action INTEGER NOT NULL,
    scan INTEGER NOT NULL,
    key INTEGER,
    ALTER TABLE KeyEvent ADD COLUMN symbol TEXT,
    FOREIGN KEY(device) REFERENCES Device(id)
);

)",
};

#endif
