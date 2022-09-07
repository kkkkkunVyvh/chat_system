#include "chatlist.h"
#include "ui_chatlist.h"

Chatlist::Chatlist(QTcpSocket *s,QString fri,QString group,QString u,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Chatlist)
{
    ui->setupUi(this);

    this->userName=u;
    socket=s;
    connect(socket,&QTcpSocket::readyRead,this,&Chatlist::server_reply);

    //根据 | 解析字符串，得到一个string链表
    QStringList friList=fri.split('|');

    //遍历链表
    for(int i=0;i<friList.size();i++)
    {
        //向好友列表中添加好友
        if(friList.at(i)!="")
        {
            ui->friendList->addItem(friList.at(i));
        }
    }

    QStringList groList=group.split('|');
    for(int i=0;i<groList.size();i++)
    {
        //向群聊列表中添加群聊
        if(groList.at(i)!="")
        {
            ui->groupList->addItem(groList.at(i));
        }
    }

    //私聊(在friendList中，双击)
    connect(ui->friendList,&QListWidget::itemDoubleClicked,
            this,&Chatlist::on_friendList_double_clicked);

    //群聊
    connect(ui->groupList,&QListWidget::itemDoubleClicked,
            this,&Chatlist::on_groupList_double_clicked);
}

Chatlist::~Chatlist()
{
    delete ui;
}

void Chatlist::server_reply()
{
    //服务器转发数据过来，把所有数据拿过来
    QByteArray ba=socket->readAll();

    //转化为obj对象
    QJsonObject obj=QJsonDocument::fromJson(ba).object();

    //判断cmd，执行响应操作
    QString cmd=obj.value("cmd").toString();
    if(cmd=="friend_login")
    {
        //好友上线提醒响应
        client_login_reply(obj.value("friend").toString());
    }
    else if(cmd=="add_reply")
    {
        //添加好友响应
        client_add_friend_reply(obj);
    }
    else if(cmd=="add_friend_reply")
    {
        //对方添加你为好友响应
        QString str=QString("%1把你添加为好友").arg(obj.value("result").toString());
        QMessageBox::information(this,"添加好友提醒",str);
        ui->friendList->addItem(obj.value("result").toString());
    }
    else if(cmd=="create_group_reply")
    {
        //创建群聊响应
        client_create_group_reply(obj);
    }
    else if(cmd=="add_group_reply")
    {
        client_add_group_reply(obj);
    }
    else if(cmd=="private_chat_reply")
    {
        //聊天时对方不在线
        client_private_chat_reply(obj.value("result").toString());
    }
    else if(cmd=="private_chat")
    {
        //聊天
        client_chat_reply(obj);
    }
    else if(cmd=="get_group_member_reply")
    {
        //获取群成员
        client_get_group_member_reply(obj);
    }
    else if(cmd=="group_chat")
    {
        //群聊
        client_group_chat_reply(obj);
    }
}

//好友上线提醒响应
void Chatlist::client_login_reply(QString fri)
{
    //把传进来的姓名与 上线了合在一起
    QString str=QString("%1 上线了").arg(fri);
    QMessageBox::information(this,"好友上线提醒",str);
}

//单击添加好友事件
void Chatlist::on_addButton_clicked()
{
    Addfriend *addFriendWidget=new Addfriend(socket,userName);
    addFriendWidget->show();
}

//添加好友响应
void Chatlist::client_add_friend_reply(QJsonObject &obj)
{
    if(obj.value("result").toString()=="user_not_exist")
    {
        QMessageBox::warning(this,"添加好友提醒","好友不存在");
    }
    else if(obj.value("result").toString()=="already_friend")
    {
        QMessageBox::warning(this,"添加好友提醒","已经为您的好友");
    }
    else if(obj.value("result").toString()=="success")
    {
        QMessageBox::information(this,"添加好友提醒","添加成功");
        ui->friendList->addItem(obj.value("friend").toString());
    }
}

//点击创建群聊事件
void Chatlist::on_createGroupButton_clicked()
{
    CreateGroup *createGroupWidget=new CreateGroup(socket,userName);
    createGroupWidget->show();
}

//创建群聊响应
void Chatlist::client_create_group_reply(QJsonObject &obj)
{
    if(obj.value("result").toString()=="group_exist")
    {
        QMessageBox::warning(this,"创建群提示","群已经存在");
    }
    else if(obj.value("result").toString()=="success")
    {
        ui->groupList->addItem(obj.value("group").toString());
    }

}

//单击添加群聊按钮
void Chatlist::on_addGroupButton_clicked()
{
    AddGroup *addGroupWidget=new AddGroup(socket,userName);
    addGroupWidget->show();
}

//添加群聊响应
void Chatlist::client_add_group_reply(QJsonObject &obj)
{
    if(obj.value("result").toString()=="group_not_exist")
    {
        QMessageBox::warning(this,"添加群提示","群不存在");
    }
    else if(obj.value("result").toString()=="user_in_group")
    {
        QMessageBox::warning(this,"添加群提示","宁已经在群中");
    }
    else if(obj.value("result").toString()=="success")
    {
        ui->groupList->addItem(obj.value("group").toString());
    }

}

//双击好友名称事件
void Chatlist::on_friendList_double_clicked()
{
    //点击谁的名字就会获取谁
    QString friendName=ui->friendList->currentItem()->text();
    PrivateChat *privateChatWidget = new PrivateChat(socket,userName,friendName,this,&chatWidgetList);
    privateChatWidget->setWindowTitle(friendName);
    privateChatWidget->show();

    ChatWidgetInfo c={privateChatWidget,friendName};
    chatWidgetList.push_back(c);
}

void Chatlist::client_private_chat_reply(QString  res)
{
    if(res=="offline")
    {
        QMessageBox::warning(this,"发送提醒","对方不在线");
    }
}

//对方发信息过来时响应
void Chatlist::client_chat_reply(QJsonObject &obj)
{
    int flag=0;
    for (int i=0; i<chatWidgetList.size();i++)
    {
        //判断有没有打开对方的聊天窗
        if(chatWidgetList.at(i).name==obj.value("user_from").toString())
        {
            flag=1;
            break;
        }
    }

    //聊天窗口没有打开
    if(flag==0)
    {
        PrivateChat *privateChatWidget = new PrivateChat(socket,userName,obj.value("user_from").toString(),this,&chatWidgetList);
        privateChatWidget->setWindowTitle(obj.value("user_from").toString());
        privateChatWidget->show();

        ChatWidgetInfo c={privateChatWidget,obj.value("user_from").toString()};
        chatWidgetList.push_back(c);
    }

    emit signals_to_sub_widget(obj);
}

void Chatlist::on_groupList_double_clicked()
{
    QString groupName=ui->groupList->currentItem()->text();
    GroupChat *groupChatWidget=new GroupChat(socket,groupName,userName,this,&groupWidgetList);
    groupChatWidget->setWindowTitle(groupName);
    groupChatWidget->show();

    GroupWidgetInfo g={groupChatWidget,groupName};
    groupWidgetList.push_back(g);

}

void Chatlist::client_get_group_member_reply(QJsonObject obj)
{
    emit signals_to_sub_widget_member(obj);

}

//群聊
void Chatlist::client_group_chat_reply(QJsonObject obj)
{
    int flag=0;
    for(int i=0;i<groupWidgetList.size();i++)
    {
        if(groupWidgetList.at(i).name==obj.value("group").toString())
        {
            flag=1;
            break;
        }
    }

    if(flag==0)
    {
        QString groupName=obj.value("group").toString();
        GroupChat *groupChatWidget=new GroupChat(socket,groupName,userName,this,&groupWidgetList);
        groupChatWidget->setWindowTitle(groupName);
        groupChatWidget->show();

        GroupWidgetInfo g={groupChatWidget,groupName};
        groupWidgetList.push_back(g);
    }

    emit signals_to_sub_widget_group(obj);
}
