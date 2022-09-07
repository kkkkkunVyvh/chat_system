#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    //创建socket连接tcp服务器
    socket=new QTcpSocket;
    socket->connectToHost(QHostAddress("192.168.255.129"),8000);

    //槽连接

    connect(socket,&QTcpSocket::connected,this,&Widget::connect_success);

    //监听服务器发过来的信息，调用server_reply
    connect(socket,&QTcpSocket::readyRead,this,&Widget::server_reply);
}

Widget::~Widget()
{
    delete ui;
}

//连接成功提示
void Widget::connect_success()
{
    QMessageBox::information(this,"连接提示","连接服务器成功");
}

//单击注册按钮事件
void Widget::on_registerButton_clicked()
{
    //获取文本
    QString username= ui->userLineEdit->text();

    //获取密码
    QString password=ui->passwdLineEdit->text();

    //封装json
    QJsonObject obj;
    obj.insert("cmd","register");
    obj.insert("user",username);
    obj.insert("password",password);

    //转换格式
    QByteArray ba=QJsonDocument(obj).toJson();

    //发给客户端
    socket->write(ba);
}

//单击登录按钮事件
void Widget::on_loginButton_clicked()
{
    QString username=ui->userLineEdit->text();

    QString password=ui->passwdLineEdit->text();

    QJsonObject obj;
    obj.insert("cmd","login");
    obj.insert("user",username);
    obj.insert("password",password);

    //把输入框的值传给成员变量
    userName=username;

    QByteArray ba=QJsonDocument(obj).toJson();

    socket->write(ba);
}


//接收服务器响应
void Widget::server_reply()
{
    //服务器转发数据过来，把所有数据拿过来
    QByteArray ba=socket->readAll();

    //转化为obj对象
    QJsonObject obj=QJsonDocument::fromJson(ba).object();

    //判断cmd，执行响应操作
    QString cmd=obj.value("cmd").toString();
    if(cmd=="register_reply")
    {
        //注册
        client_register_handler(obj.value("result").toString());
    }
    else if(cmd=="login_reply")
    {
        //登录
        client_login_handler(obj.value("result").toString(),
                             obj.value("friend").toString(),
                             obj.value("group").toString());
    }
}

//注册响应
void Widget::client_register_handler(QString res)
{
    if(res=="success")
    {
        //提示框
        QMessageBox::information(this,"注册提示","注册成功");
    }
    else if(res=="failure")
    {
        //警告框
        QMessageBox::warning(this,"注册提示","注册失败");
    }
}

//登录响应
void Widget::client_login_handler(QString res,QString fri,QString group)
{
    if(res=="user_not_exist")
    {
        QMessageBox::warning(this,"登录提示","用户名不存在");
    }
    else if(res=="password_error")
    {
        QMessageBox::warning(this,"登录提示","密码错误");
    }
    else if(res=="success")
    {
        //隐藏原来的窗口
        this->hide();
        //结束信号与槽的关系
        socket->disconnect(SIGNAL(readyRead()));

        //跳转到新窗口
        Chatlist *c=new Chatlist(socket,fri,group,userName);

        //设置标题
        c->setWindowTitle(userName);

        c->show();
    }
}

