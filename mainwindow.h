#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>

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

private slots:
    void addComboBoxItem(QString item);
    void delComboBoxItem();
    void on_pushButton_clicked();
    void requestFinished(const QString& str);

    void on_cleanButton_clicked();

    void on_postButton_clicked();

    void on_tokenButton_clicked();

private:
    Ui::MainWindow *ui;
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
