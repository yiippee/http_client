#include "mainwindow.h"
#include <QApplication>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>

void testSql()
{
    //创建连接
   QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");//第二个参数可以设置连接名字，这里为default

   db.setDatabaseName( "./testdatabase.db" );// 设置数据库名与路径, 此时是放在上一个目录
   //打开连接
   if( !db.open() )
   {
     qDebug() << db.lastError();
     qFatal( "Failed to connect." );
   }

   //各种操作
   // QSqlQuery qry(db);
   QSqlQuery qry;

   //创建table
//   qry.prepare( "CREATE TABLE IF NOT EXISTS names (id INTEGER UNIQUE PRIMARY KEY, firstname VARCHAR(30), lastname VARCHAR(30))" );
//   if( !qry.exec() )
//     qDebug() << qry.lastError();
//   else
//     qDebug() << "Table created!";

//   //增
//    qry.prepare( "INSERT INTO names (id, firstname, lastname) VALUES (1, 'John', 'Doe')" );
//    if( !qry.exec() )
//      qDebug() << qry.lastError();
//    else
//      qDebug( "Inserted!" );


    //查询
      qry.prepare( "SELECT * FROM names" );
      if( !qry.exec() )
        qDebug() << qry.lastError();
      else
      {
        qDebug( "Selected!" );

        QSqlRecord rec = qry.record();

        int cols = rec.count();

        for( int c=0; c<cols; c++ )
          qDebug() << QString( "Column %1: %2" ).arg( c ).arg( rec.fieldName(c) );

        for( int r=0; qry.next(); r++ )
          for( int c=0; c<cols; c++ )
            qDebug() << QString( "Row %1, %2: %3" ).arg( r ).arg( rec.fieldName(c) ).arg( qry.value(c).toString() );
      }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    return a.exec();
}
