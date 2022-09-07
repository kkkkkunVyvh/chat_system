#include "addfriend.h"
#include "ui_addfriend.h"

Addfriend::Addfriend(QTcpSocket *s, QString u, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Addfriend)
{
    ui->setupUi(this);

    this->userName=u;
    this->socket=s;
}

Addfriend::~Addfriend()
{
    delete ui;
}

void Addfriend::on_cancelButton_clicked()
{
    //关闭这个窗口
    this->close();
}

//单击添加好友事件
void Addfriend::on_addButton_clicked()
{
    QString friendName= ui->lineEdit->text();
    QJsonObject obj;
    obj.insert("cmd","add");
    obj.insert("user",userName);
    obj.insert("friend",friendName);

    QByteArray ba=QJsonDocument(obj).toJson();
    socket->write(ba);

    this->close();
}
