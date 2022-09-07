#ifndef CHATLIST_H
#define CHATLIST_H

#include <QWidget>

#include<QTcpSocket>
#include<QJsonObject>
#include<QJsonDocument>
#include<QMessageBox>
#include<QList>
#include"addfriend.h"
#include"creategroup.h"
#include"addgroup.h"
class GroupChat;
#include"groupchat.h"

class PrivateChat;
#include"privatechat.h"

namespace Ui {
class Chatlist;
}

struct ChatWidgetInfo
{
    PrivateChat *w;
    QString name;
};

struct GroupWidgetInfo
{
    GroupChat *w;
    QString name;
};

class Chatlist : public QWidget
{
    Q_OBJECT

public:
    explicit Chatlist(QTcpSocket *,QString,QString,QString,QWidget *parent = nullptr);
    ~Chatlist();

private slots:
    void server_reply();

    void on_addButton_clicked();

    void on_createGroupButton_clicked();

    void on_addGroupButton_clicked();

    void on_friendList_double_clicked();

    void on_groupList_double_clicked();

signals:
    //
    void signals_to_sub_widget(QJsonObject);

    //获取群成员信号
    void signals_to_sub_widget_member(QJsonObject);

    //
    void signals_to_sub_widget_group(QJsonObject);

private:
    //好友上线提醒响应
    void client_login_reply(QString);

    //添加好友响应
    void client_add_friend_reply(QJsonObject &obj);

    //创建群聊响应
    void client_create_group_reply(QJsonObject &obj);

    //添加群聊响应
    void client_add_group_reply(QJsonObject &obj);

    //私聊时对方不在线响应
    void client_private_chat_reply(QString);

    //对方发信息过来时响应
    void client_chat_reply(QJsonObject &obj);

    //获取群成员
    void client_get_group_member_reply(QJsonObject obj);

    //群聊
    void client_group_chat_reply(QJsonObject obj);

    Ui::Chatlist *ui;

    QTcpSocket *socket;

    QString userName;

    QList<ChatWidgetInfo> chatWidgetList;

    QList<GroupWidgetInfo> groupWidgetList;


};

#endif // CHATLIST_H
