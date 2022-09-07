#ifndef CHAT_DATABASE_H
#define CHAT_DATABASE_H

#include<iostream>
#include<string.h>
#include<mysql/mysql.h>
#include<stdio.h>


using namespace std;

class ChatDatabase
{
private:
	MYSQL *mysql;

public:
	ChatDatabase();
	~ChatDatabase();

	//连接数据库(传入数据库名）
	void my_database_connect(const char * name);

	//获取群个数，返回到一个string*数组里面去
	int my_database_get_group_name(string *);

	//获取群的成员(表名，要传入的string字符串)
	void my_database_get_group_member(string,string &);
	
	//判断用户是否存在
	bool my_database_user_exist(string);

	//断开连接
	void my_database_disconnect();

	//注册
	void my_database_user_password(string ,string);
	
	//获取好友列表
	void my_database_get_friend_group(string,string &,string &);

	//判断密码对错
	bool my_database_password_correct(string ,string);

	//判断两个人是否为好友关系
	bool my_database_is_friend(string,string);

	//添加好友(在第一个中添加第二个)
	void my_database_add_new_friend(string,string);

	//判断群是否存在
	bool my_database_group_exist(string);

	//建群
	void mydatabase_add_new_group(string,string);
	
	//用户群列表中加入群
	void my_database_user_add_group(string ,string);
	
	//群的用户列表加入用户
	void my_database_group_add_user(string ,string);
};




#endif
