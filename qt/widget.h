#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

#include <QTcpSocket>
#include<QHostAddress>
#include<QMessageBox>
#include<QJsonObject>
#include<QJsonDocument>
#include"chatlist.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:

    //槽连接
    void connect_success();
    void server_reply();

    //点击注册按钮事件
    void on_registerButton_clicked();

    //点击登录按钮事件
    void on_loginButton_clicked();

private:
    Ui::Widget *ui;

    //socket对象
    QTcpSocket *socket;

    QString userName;

    //注册响应
    void client_register_handler(QString);

    //登录响应
    void client_login_handler(QString,QString,QString);


};
#endif // WIDGET_H
