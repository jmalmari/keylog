#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <chrono>
#include <string.h>
#include <exception>
#include "../input/keyevent.hpp"
#include "../input/symbolmap.hpp"
#include "keylog.hpp"
#include "databasesql.hpp"

using namespace std::chrono;

KeyLog::KeyLog(std::string const& dbname,
               std::string const& deviceName) :
    _dbVersion(0),
    _deviceId(-1)
{
    sqlite3* pdb;
    int status = sqlite3_open(dbname.c_str(), &pdb);

    if (status != SQLITE_OK)
    {
        throw std::runtime_error("couldn't open database");
    }

    _db.reset(pdb);

    migrate();

    _deviceId = initDevice(deviceName);
}

KeyLog::~KeyLog()
{
}

void KeyLog::migrate()
{
    int vfrom = dbVersion();
    _dbVersion = vfrom;

    while (vfrom < SqlMigrationSteps.size())
    {
        if (!executeSql("BEGIN EXCLUSIVE TRANSACTION"))
        {
            std::cerr << "Couldn't obtain exclusive lock on database.\n";
            return;
        }

        if (executeSql(SqlMigrationSteps[vfrom]))
        {
            (void)executeSql("COMMIT");
        }
        else
        {
            (void)executeSql("ROLLBACK");
        }

        _dbVersion = dbVersion();
        int vto = _dbVersion;

        if (vto <= vfrom)
        {
            throw std::runtime_error("migration step failed");
        }

        std::cout << "migrated database from version "
                  << vfrom << " to " << vto << std::endl;

        // prepare for next step
        vfrom = vto;
    }
}

int KeyLog::dbVersion()
{
    int version;
    if (executeSqlScalar("PRAGMA user_version", version))
    {
        return version;
    }
    else
    {
        return 0;
    }
}

bool KeyLog::executeSqlScalar(std::string const& sql, int& result)
{
    sqlite3_stmt* p;
    int status = sqlite3_prepare_v2(_db.get(),
                                    sql.c_str(), -1,
                                    &p, nullptr);

    std::unique_ptr<sqlite3_stmt, StmtDeleter> stmt(p);
    p = nullptr;

    if (status != SQLITE_OK)
    {
        throw std::runtime_error("sql query prepare failed");
    }

    status = sqlite3_step(stmt.get());

    if (status == SQLITE_DONE)
    {
        return false;
    }

    if (status != SQLITE_ROW)
    {
        throw std::runtime_error("got other than SQLITE_ROW");
    }

    result = sqlite3_column_int(stmt.get(), 0);

    status = sqlite3_step(stmt.get());

    if (status != SQLITE_DONE)
    {
        throw std::runtime_error("expected SQLITE_DONE");
    }

    return true;
}

bool KeyLog::executeSql(std::string const& sql)
{
    char* errMsg;
    int status = sqlite3_exec(_db.get(), sql.c_str(),
                              nullptr, nullptr, &errMsg);

    if (status != SQLITE_OK)
    {
        std::cerr << "db error: " << errMsg << "\n";
        sqlite3_free(errMsg);
        errMsg = nullptr;
    }

    return status == SQLITE_OK;
}

int KeyLog::initDevice(std::string const& name)
{
    std::ostringstream idQuery;
    idQuery << "SELECT id FROM Device WHERE name='"
        << name << "';";

    int id;
    if (executeSqlScalar(idQuery.str(), id))
    {
        return id;
    }
    else
    {
        std::ostringstream idInsert;
        idInsert << "INSERT INTO Device (name) VALUES ('"
            << name << "');";

        if (executeSql(idInsert.str()) && executeSqlScalar(idQuery.str(), id))
        {
            return id;
        }
    }

    std::cerr << "couldn't get device id\n";
    return -1;
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

    std::cout
        << event.action
        << "\t" << std::dec << event.scancode
        << " = 0x" << std::hex << event.scancode << std::dec
        << " (keycode " << event.key << ")"
        << ", " << SymbolNames.at(event.key % SymbolNames.size())
        << std::endl;

    if (_dbVersion != DatabaseVersion)
    {
        std::cerr << "Database is at version " << _dbVersion
                  << ", migration needed to version " << DatabaseVersion
                  << std::endl;
        return;
    }

    if (_deviceId < 0)
    {
        std::cerr << "No device id\n";
        return;
    }

    std::ostringstream sql;
    sql << "INSERT INTO KeyEvent (device, timestamp, action, scan, key) VALUES ("
        << _deviceId << ", "
        << ms.time_since_epoch().count() << ", "
        << static_cast<int>(event.action) << ", "
        << event.scancode << ", ";
    if (event.key < 0)
    {
        sql << "NULL";
    }
    else
    {
        sql << event.key;
    }

    sql << ");";


    char* errMsg(nullptr);
    int status = sqlite3_exec(_db.get(), sql.str().c_str(), nullptr, nullptr, &errMsg);

    if (status != SQLITE_OK)
    {
        std::cerr << "sqlite error: " << errMsg << "\n";
        sqlite3_free(errMsg);
    }
}
