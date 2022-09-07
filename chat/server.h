#ifndef SERVER_H
#define SERVER_H

#include<event.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<thread>
#include<iostream>
#include<event2/listener.h>
#include"chatlist.h"
#include<jsoncpp/json/json.h>
#include<unistd.h>

using namespace std;

#define IP "192.168.255.129"
#define PORT  8000

class Server 
{
private: 
	struct event_base *base;  //事件集合
	struct evconnlistener *listener;   //监听事件
	static ChatInfo *chatlist;                 //链表对象 含有两个链表
	static ChatDatabase *chatdb;            //数据库对象
private:
	//监听客户端
	static void listener_cb(struct evconnlistener *listener,evutil_socket_t fd,struct sockaddr *addr,int socklen,void *arg);
	
	//处理客户端请求
	static void client_handler(int);

	//处理文件传输请求
	static void send_file_handler(int,int,int*,int*);
	
	//接受客户端发过来的信息
	static void read_cb(struct bufferevent *bev,void *ctx);
	
	//异常处理
	static void event_cb(struct bufferevent *bev,short what,void *ctx);

	//注册功能
        static void server_register(struct bufferevent *bev,Json::Value val);

	//登录功能
        static void server_login(struct bufferevent *bev,Json::Value val);

	//添加好友功能
        static void server_add_friend(struct bufferevent *bev,Json::Value val);
	
	//建群
	static void server_create_group(struct bufferevent *bev,Json::Value val);

	//添加群
	static void server_add_group(struct bufferevent *bev,Json::Value val);
		
	//私聊
        static void server_private_chat(struct bufferevent *bev,Json::Value val);

	//群聊
        static void server_group_chat(struct bufferevent *bev,Json::Value val);

	//获取群成员
        static void server_get_group_member(struct bufferevent *bev,Json::Value val);

	//用户下线
        static void server_user_offline(struct bufferevent *bev,Json::Value val);

	//发送文件
        static void server_send_file(struct bufferevent *bev,Json::Value val);


public:
	Server(const char *ip="127.0.0.1",int port=8000);
	~Server();
};



#endif
