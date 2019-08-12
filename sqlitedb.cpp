#include "sqlitedb.h"
#include <QtSql/QSqlError>
#include <QDebug>

SqliteDB* SqliteDB::instance = nullptr;

SqliteDB::SqliteDB()
{
    //创建连接
    db = QSqlDatabase::addDatabase("QSQLITE");//第二个参数可以设置连接名字，这里为default

    db.setDatabaseName( "./sqlitedb.db" );// 设置数据库名与路径, 此时是放在上一个目录
    //打开连接
    if( !db.open() )
    {
        qDebug() << db.lastError();
        qFatal( "Failed to connect." );
    }
}
