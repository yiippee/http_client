#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QCloseEvent>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static void handle (const QString & str);
    bool login(const QString &userName, const QString &passwd);
    void sendLoginRequest(const QString &userName, const QString &passwd);

signals:
    void logined(bool, const QString&);
    void logined2();

private slots:
    void addComboBoxItem(QString item);
    void delComboBoxItem();
    void on_pushButton_clicked();
    void requestFinished(const QString& str);

    void on_cleanButton_clicked();

    void on_postButton_clicked();

    void on_tokenButton_clicked();

    void receiveWsMsg(const QString& msg);

    void on_actionabout_triggered();

    void on_websocketButton_clicked();

private:
    void disPlayTextBrowser(const QString& content);
    Ui::MainWindow *ui;

protected:
    //这是一个虚函数，继承自QEvent.只要重写了这个虚函数，当你按下窗口右上角的"×"时，就会调用你所重写的此函数.
    virtual void closeEvent(QCloseEvent*event);
};

class http:public QObject {

        Q_OBJECT
public:
    http() {
    }
    ~http() {
    }
private:
    QString token;

signals:
    void valueChanged(const QString& str);

public:
    void send(const QString& str);
    static void handleToken (const QString & str);
    void SetToken(const QString& str);
    QString& GetToken();
};
#endif // MAINWINDOW_H
