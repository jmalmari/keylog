#ifndef KEYLOG_HPP
#define KEYLOG_HPP

#include <cstdint>
#include <map>
#include <ostream>
#include <memory>
#include <sqlite3.h>
#include "../input/iinputlistener.hpp"

struct DbDeleter
{
    void operator()(sqlite3* db)
    {
        if (db)
        {
            sqlite3_close(db);
        }
    }
};

struct StmtDeleter
{
    void operator()(sqlite3_stmt* stmt)
    {
        (void)sqlite3_finalize(stmt);
    }
};

class KeyLog : public IInputListener
{
public:
    KeyLog(std::string const& dbname,
           std::string const& deviceName);
    virtual ~KeyLog();

    void printStats(std::ostream& stream) const;

protected:
    void onKeyEvent(KeyEvent const& event);

private:
    void migrate();
    int dbVersion();
    bool executeSql(std::string const& sql);
    bool executeSqlScalar(std::string const& sql, int& result);
    int initDevice(std::string const& name);
    char const* symbolName(int keycode) const;

private:
    std::unique_ptr<sqlite3, DbDeleter> _db;
    int _dbVersion;
    int _deviceId;
};

#endif
