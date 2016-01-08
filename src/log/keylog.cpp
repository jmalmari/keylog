#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <chrono>
#include <string.h>
#include <exception>
#include "../input/keyevent.hpp"
#include "keylog.hpp"

namespace {

int const CurrentDatabaseVersion = 1;

char const* const SqlInitScript = R"(
PRAGMA user_version = 1;

CREATE TABLE KeyEvent (
    id INTEGER PRIMARY KEY,
    key INTEGER,
    action INTEGER,
    timestamp INTEGER
)
)";

}

using namespace std::chrono;

KeyLog::KeyLog(std::string const& dbname)
{
    sqlite3* pdb;
    int status = sqlite3_open(dbname.c_str(), &pdb);

    if (status != SQLITE_OK)
    {
        throw std::runtime_error("couldn't open database");
    }

    _db.reset(pdb);

    migrate();
}

KeyLog::~KeyLog()
{
}

void KeyLog::migrate()
{
    while (migrateStep())
    {
        std::cout << "migrated to version " << dbVersion() << std::endl;
    }
}

bool KeyLog::migrateStep()
{
    switch (dbVersion())
    {
    case 0:
        createDatabase();
        return true;
    case 1:
        return false;
    default:
        throw std::runtime_error("unknown database version");
    }
}

int KeyLog::dbVersion()
{
    sqlite3_stmt* stmt;
    int status = sqlite3_prepare_v2(_db.get(),
                                    "PRAGMA user_version", -1,
                                    &stmt, nullptr);

    if (status != SQLITE_OK)
    {
        throw std::runtime_error("user version query prepare failed");
    }

    status = sqlite3_step(stmt);

    if (status != SQLITE_ROW)
    {
        throw std::runtime_error("got other than SQLITE_ROW");
    }

    int version = sqlite3_column_int(stmt, 0);

    status = sqlite3_step(stmt);

    if (status != SQLITE_DONE)
    {
        throw std::runtime_error("expected SQLITE_DONE");
    }

    status = sqlite3_finalize(stmt);

    if (status != SQLITE_OK)
    {
        throw std::runtime_error("statement finalize failed");
    }

    return version;
}

void KeyLog::createDatabase()
{
    char* errMsg;
    int status = sqlite3_exec(_db.get(), SqlInitScript,
                              nullptr, nullptr, &errMsg);

    if (status != SQLITE_OK)
    {
        std::cerr << "db error: " << errMsg << "\n";
        sqlite3_free(errMsg);
        errMsg = nullptr;
    }
}

void KeyLog::printStats(std::ostream&) const
{
    // stream << "keylog stats not available atm\n";
}

void KeyLog::onKeyEvent(KeyEvent const& event)
{
    if (event.scancode < 0)
    {
        std::cerr << "unsupported scan code " << event.scancode << '\n';
        return;
    }

    if (event.action != KeyEvent::KeyPressed &&
        event.action != KeyEvent::KeyReleased)
    {
        return;
    }

    time_point<steady_clock, milliseconds> ms(
        time_point_cast<milliseconds>(steady_clock::now()));

    std::ostringstream sql;
    sql <<
        "INSERT INTO KeyEvent (key, action, timestamp) "
        "VALUES ("
        << event.scancode << ", "
        << event.action << ", "
        << ms.time_since_epoch().count()
        << ")";

    std::cout << sql.str() << std::endl;

    char* errMsg(nullptr);
    int status = sqlite3_exec(_db.get(), sql.str().c_str(), nullptr, nullptr, &errMsg);

    if (status != SQLITE_OK)
    {
        std::cerr << "sqlite error: " << errMsg << "\n";
        sqlite3_free(errMsg);
    }
}
