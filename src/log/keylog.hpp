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
        sqlite3_finalize(stmt);
    }
};

class KeyLog : public IInputListener
{
public:
    KeyLog(std::string const& dbname);
    virtual ~KeyLog();

    void printStats(std::ostream& stream) const;

protected:
    void onKeyEvent(KeyEvent const& event);

private:
    void migrate();
    bool migrateStep();
    int dbVersion();
    void createDatabase();

private:
    std::unique_ptr<sqlite3, DbDeleter> _db;
};

#endif
