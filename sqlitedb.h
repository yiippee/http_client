#ifndef SQLITEDB_H
#define SQLITEDB_H
#include <QtSql/QSqlDatabase>

class SqliteDB
{
public:
    static SqliteDB* getInstance()
    {
        if(instance == nullptr)
            instance = new SqliteDB();
        return instance;
    }
    QSqlDatabase db;
private:
    static SqliteDB* instance;
    QString dbName;

    SqliteDB();
    ~SqliteDB();
    SqliteDB(const SqliteDB&);
    SqliteDB& operator=(const SqliteDB&);
};

#endif // SQLITEDB_H
