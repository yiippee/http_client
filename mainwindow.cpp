#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "httpclient.h"
#include "sqlitedb.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRecord>

http h;

void http::send(const QString& str) {
    emit valueChanged(str);
}

void http::SetToken(const QString& str) {
    token = str;
    qDebug() << "token:" << token;

    QSqlQuery qry(SqliteDB::getInstance()->db);
    qry.prepare( "CREATE TABLE IF NOT EXISTS token (id INTEGER UNIQUE PRIMARY KEY, token VARCHAR(1024), val VARCHAR(10))" );
    if( !qry.exec() )
        qDebug() << qry.lastError();
    else
        qDebug() << "Table created!";

    //增
    qry.prepare( "INSERT INTO token (token, val) VALUES (?, ?)" );
    qry.bindValue(0, token);
    qry.bindValue(1, token);
    if( !qry.exec() )
      qDebug() << qry.lastError();
    else
      qDebug( "Inserted!" );
}

QString& http::GetToken()
{
    if (token == "") {
        QSqlDatabase db = SqliteDB::getInstance()->db;
        QSqlQuery qry(db);
        //查询
        qry.prepare( "SELECT token FROM token order by id desc limit 1" );
        if( !qry.exec() )
            qDebug() << qry.lastError();
        else
        {
            qDebug( "Selected!" );
            qry.next();
            token = qry.value(0).toString();
        }
    }

    return token;
}


void http::handleToken (const QString & str) {
    QJsonObject obj;
    QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8());
    // check validity of the document
    if(!doc.isNull()){
        if(doc.isObject()){
            obj = doc.object();
        } else {
            qDebug() << "Document is not an object" << endl;
        }
    } else {
        qDebug() << "Invalid JSON...\n" << str << endl;
    }
    if(obj.contains("token")) {
        QJsonValue tokenValue = obj.take("token");
        if(tokenValue.isString()) {
            h.SetToken(tokenValue.toString());
        }
    }

    h.send(str);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->comboBox->setEditable(true);
    QAction *del_act = new QAction("删除");
    ui->comboBox->addAction(del_act);
    ui->comboBox->setContextMenuPolicy(Qt::ActionsContextMenu);

    // 初始化 combobox idem
    //查询
    QSqlQuery qry(SqliteDB::getInstance()->db);
    qry.prepare( "SELECT url FROM url" );
    if( !qry.exec() )
        qDebug() << qry.lastError();
    else
    {
        qDebug( "Selected!" );

        QSqlRecord rec = qry.record();

        int cols = rec.count();
        for( int r=0; qry.next(); r++ )
            for( int c=0; c<cols; c++ ) {
                QString item = qry.value(c).toString();
                addComboBoxItem(item);
            }
    }

    connect(del_act, SIGNAL(triggered()), this, SLOT(delComboBoxItem()));
    //connect(del_act, SIGNAL(triggered()), this, SLOT(addComboBoxItem()));

    QObject::connect(&h, SIGNAL(valueChanged(const QString&)),this,SLOT(requestFinished(const QString&)));
}

void MainWindow::delComboBoxItem()
{
    qDebug() << "delComboBoxItem";
    int currentidx = ui->comboBox->currentIndex();
    QString url = ui->comboBox->currentText();
    ui->comboBox->removeItem(currentidx);

    QSqlQuery qry(SqliteDB::getInstance()->db);
    //删除
    qry.prepare( "DELETE FROM url WHERE url = ?" );
    qry.bindValue(0, url);
    if( !qry.exec() )
        qDebug() << qry.lastError();
    else
        qDebug( "Deleted!" );
}

void MainWindow::addComboBoxItem(QString item)
{
    if(ui->comboBox->findText(item) == -1) // 针对addItem方法可避免重复添加
    {
       ui->comboBox->insertItem(0, item);
       ui->comboBox->setCurrentIndex(0);
       // 存数据库
       QSqlQuery qry(SqliteDB::getInstance()->db);
       qry.prepare( "CREATE TABLE IF NOT EXISTS url (id INTEGER UNIQUE PRIMARY KEY, url VARCHAR(1024), val VARCHAR(10))" );
       if( !qry.exec() )
           qDebug() << qry.lastError();
       else
           qDebug() << "Table created!";

       //增
       qry.prepare( "INSERT INTO url (url, val) VALUES (?, ?)" );
       qry.bindValue(0, item);
       qry.bindValue(1, item);
       if( !qry.exec() )
         qDebug() << qry.lastError();
       else
         qDebug( "Inserted!" );
    }
}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::handle (const QString & str) {
    qDebug() << "OK...";
    h.send(str);
}


void errorHandler(const QString& str) {
    qDebug() << "error: " << str;
    h.send("ERROR: \r\n" + str);
}

void MainWindow::on_pushButton_clicked()
{
    qDebug() << "get url";
    QString baseUrl = ui->comboBox->currentText();
    int port = ui->portEdit->text().toInt();
    HttpClient client(baseUrl, port);
    client.param("token", h.GetToken());
    client.get(this->handle, errorHandler);

    addComboBoxItem(baseUrl);
}

void MainWindow::requestFinished(const QString& str) {
    ui->textBrowser->insertPlainText(str+"\r\n");
}


void MainWindow::on_cleanButton_clicked()
{
    ui->textBrowser->clear();
}

void MainWindow::on_postButton_clicked()
{
    // QString baseUrl = ui->urlEdit->text();
    QString baseUrl = ui->comboBox->currentText();
    int port = ui->portEdit->text().toInt();
    QString usernameKey = ui->Editparam_k1->text();
    QString usernameVal = ui->Editparam_v1->text();
    QString passwordKey = ui->Editparam_k2->text();
    QString passwordVal = ui->Editparam_v2->text();

    HttpClient client(baseUrl, port);
    client.param(usernameKey, usernameVal);
    client.param(passwordKey, passwordVal);

    client.post(h.handleToken, errorHandler);

    addComboBoxItem(baseUrl);
}

void MainWindow::on_tokenButton_clicked()
{
    ui->textBrowser->insertPlainText("\r\nTOKEN:\r\n" + h.GetToken()+"\r\n");
}

