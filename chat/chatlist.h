#ifndef CHATINFO_H
#define CHATINFO_H

#include<event.h>
#include<iostream>
#include<list>
#include<string.h>
#include"chat_database.h"

using namespace std;

#define MAXNUM   1024    //表示群的最大个数

//用户
struct User
{	
	string name;
	struct bufferevent *bev;
};
typedef struct User user;

//群成员
struct GroupUser
{
	string name;
};
typedef struct GroupUser groupUser;

//群
struct Group
{
	string name;
	list<GroupUser> *l;
};
typedef struct Group group;


class Server;

class ChatInfo
{
	friend class Server;
private:
	//保持所有在线的用户信息
	list<User> *online_user;

	//保存群信息
	list<Group> *group_info;

	//数据库对象
	ChatDatabase *mydatabase;
public:
	ChatInfo();
	~ChatInfo();

	//判断群是否存在
	bool info_group_exist(string);
	
	//判断用户是否在群中
	bool info_user_in_group(string,string);

	//在群链表中的list<GroupUser>中添加GroupUser
	void info_group_add_user(string,string);
	
	//获取需要聊天的人的bufferevent指针
	struct bufferevent *info_get_friend_bev(string);
	
	//获取群成员
	string info_get_group_member(string);
	
	void info_add_new_group(string,string);
};



#endif
