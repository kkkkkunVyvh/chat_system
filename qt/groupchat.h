#ifndef GROUPCHAT_H
#define GROUPCHAT_H

#include <QWidget>

#include<QJsonDocument>
#include<QTcpSocket>
#include<QJsonObject>
#include<QList>

class Chatlist;
struct GroupWidgetInfo;
#include"chatlist.h"

namespace Ui {
class GroupChat;
}

class GroupChat : public QWidget
{
    Q_OBJECT

public:
    explicit GroupChat(QTcpSocket *s,QString g,QString u,Chatlist *c,QList<GroupWidgetInfo> *l, QWidget *parent = nullptr);
    ~GroupChat();

    void closeEvent(QCloseEvent *event);

private slots:
    void show_group_member(QJsonObject);

    void on_sendButton_clicked();

    void show_group_text(QJsonObject);

private:
    Ui::GroupChat *ui;

    QTcpSocket *socket;
    QString userName;
    QString groupName;
    Chatlist *mainWidget;
    QList<GroupWidgetInfo> *groupWidgetList;
};

#endif // GROUPCHAT_H
