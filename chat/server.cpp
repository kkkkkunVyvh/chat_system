#include"server.h"

ChatDatabase *Server::chatdb=new ChatDatabase;
ChatInfo *Server::chatlist=new ChatInfo;


Server::Server(const char *ip,int port)
{	
    //创建事件集合
    base=event_base_new();

    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(port);
    server_addr.sin_addr.s_addr=inet_addr(ip);

    //创建监听对象
    listener=evconnlistener_new_bind(base,listener_cb,NULL,
        LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,10,(struct sockaddr *)&server_addr,
            sizeof(server_addr));
    
     if(NULL==listener)
    {
        cout<<"evconnlistener_new_bind error" <<endl;
    }
    cout<<"服务器初始化成功，正在监听客户端"<<endl;
    event_base_dispatch(base);


}

//当有客户端发起连接的时候，会触发该函数
void Server::listener_cb(struct evconnlistener *listener,evutil_socket_t fd,struct sockaddr *addr,int socklen,void *arg)
{
	
	cout<<"接受客户端的连接，fd="<<fd<<endl;

	//创建工作线程来处理该客户端
	thread client_thread(client_handler,fd);

	client_thread.detach();//线程分离，当线程运行结束后，自动释放资源

}


void Server::client_handler(int fd)
{
	//创建集合
	struct event_base *base=event_base_new();

	//创建bufferevent对象
	struct bufferevent *bev=bufferevent_socket_new(base,fd,BEV_OPT_CLOSE_ON_FREE);
	
	if(NULL==bev)
	{
		cout<<"bufferevent_socket_new error" <<endl;
	}

	//给bufferevent设置回调函数
	bufferevent_setcb(bev,read_cb,NULL,event_cb,NULL);

	//使能回调函数
	bufferevent_enable(bev,EV_READ);

	event_base_dispatch(base);  //监听集合（监听客户端是否有数据发送过来）

	event_base_free(base);

	cout<<"线程退出，释放集合"<<endl;
}

void Server::read_cb(struct bufferevent *bev,void *ctx)
{
	char buf[1024]={0};
	int size = bufferevent_read(bev,buf,sizeof(buf));
	if(size<0)
	{
 	cout<<"bufferevent_read error"<<endl;
	}

	cout<<buf<<endl;

	//解析json对象
	Json::Reader reader;

	//封装json对象
	Json::FastWriter writer;

	//把buf转换为json格式
	Json::Value val;
	if(!reader.parse(buf,val))
	{
		cout<<"服务器解析数据失败"<<endl;
	}

	//把cmd这个键的值拿出来并转成string类型
	string cmd = val["cmd"].asString();

	//所有功能的入口
	if(cmd=="register")
	{
		//注册功能  bev表示返回的值
		server_register(bev,val);
	}
	else if(cmd=="login")
	{
		//登录
		server_login(bev,val);
	}
	else if(cmd=="add")
	{
		//添加好友
		server_add_friend(bev,val);
	}
	else if(cmd=="create_group")
	{
		//建群
		server_create_group(bev,val);
	}
	else if(cmd=="add_group")
	{
		//添加群
		server_add_group(bev,val);
	}
	else if(cmd=="private_chat")
	{
		//私聊
		server_private_chat(bev,val);
	}
	else if(cmd=="group_chat")
        {
                //群聊
                server_group_chat(bev,val);
        }
	else if(cmd=="get_group_member")
	{
		//获取群成员
		server_get_group_member(bev,val);
	}
	else if(cmd=="offline")
	{
		//下线
		server_user_offline(bev,val);
	}
	else if(cmd=="send_file")
	{
		//发送文件
		server_send_file(bev,val);
	}	
	
}

void Server::event_cb(struct bufferevent *bev,short what,void *ctx)
{

}

Server::~Server()
{
	event_base_free(base);
}

//注册
void Server::server_register(struct bufferevent *bev,Json::Value val)
{
	chatdb->my_database_connect("user");
	

	if(chatdb->my_database_user_exist(val["user"].asString()))
	{
		//用户存在
		//返回客户端	
		Json::Value val;
		val["cmd"]="register_reply";
		val["result"]="failure";

		//返回json
		string s=Json::FastWriter().write(val);
		if(bufferevent_write(bev,s.c_str(),strlen(s.c_str()))<0)
		{
			cout<<"bufferevent write"<<endl;
		}	
	}
	else
	{ 	
		//用户不存在
		chatdb->my_database_user_password(val["user"].asString(),val["password"].asString());
		
		//返回客户端
		Json::Value val1;
                val1["cmd"]="register_reply";
                val1["result"]="success";

                //返回json
                string s=Json::FastWriter().write(val1);
                if(bufferevent_write(bev,s.c_str(),strlen(s.c_str()))<0)
                {
                        cout<<"bufferevent write"<<endl;
                }
    	} 
	
	chatdb->my_database_disconnect();
}

//登录
void Server::server_login(struct bufferevent *bev,Json::Value val)
{
	chatdb->my_database_connect("user");

	//用户不存在
        if(!chatdb->my_database_user_exist(val["user"].asString()))
	{
                Json::Value val;
                val["cmd"]="login_reply";
                val["result"]="user_not_exist";

                //返回json
                string s=Json::FastWriter().write(val);
                if(bufferevent_write(bev,s.c_str(),strlen(s.c_str()))<0)
                {
                        cout<<"bufferevent write"<<endl;
                }
                return;
	}  //密码不正确
	if(!chatdb->my_database_password_correct(val["user"].asString(),val["password"].asString()))
	{
		Json::Value val;
                val["cmd"]="login_reply";
                val["result"]="password_error";

                //返回json
                string s=Json::FastWriter().write(val);
                if(bufferevent_write(bev,s.c_str(),strlen(s.c_str()))<0)
                {
                        cout<<"bufferevent write"<<endl;
                }
                return;

	}
	Json::Value v;
	string s,name;
	
	//向在线用户链表中添加用户
	User u={val["user"].asString(),bev};
	chatlist->online_user->push_back(u);

	//获取好友列表以及群列表
	string friend_list,group_list;
	chatdb->my_database_get_friend_group(val["user"].asString(),friend_list,group_list);
	
	//登录成功，返回好友列表
        v["cmd"]="login_reply";
        v["result"]="success";
        v["friend"]=friend_list;
        v["group"]=group_list;


        //发送给客户端
        s=Json::FastWriter().write(v);
        if(bufferevent_write(bev,s.c_str(),strlen(s.c_str()))<0)
        {
               cout<<"bufferevent write"<<endl;
        }


	//好友上线提醒
	int start =0;    //字符串下标
        int end = 0;    //字符串下标
	int flag=1;
	while(flag)
        {
                //find(要查找的字符串，从start下标开始查找) 返回| 的下标
                end=friend_list.find("|",start);

                //结束跳出循环
                if(end ==-1)
                {
                        name=friend_list.substr(start,friend_list.size()- start);
			flag=0;
                }
		else
		{
                	//substr(从什么时候开始读，读几个)end-start 表示2-0=2，读两个字，返回读到的字符串
                	string name=friend_list.substr(start,end - start);
		}

		//遍历链表                
		for(list<User>::iterator it=chatlist->online_user->begin();it!=chatlist->online_user->end();it++)
		{
			if(name==it->name)
			{
				 v.clear();
                                 v["cmd"]="friend_login";
     		  		 v["friend"]=val["user"];

    				 //发送给客户端
			         s=Json::FastWriter().write(v);
       				 if(bufferevent_write(it->bev,s.c_str(),strlen(s.c_str()))<0)
			         {
              				 cout<<"bufferevent write"<<endl;
      	                         }

			}
		}			

                //下一次从end+1的坐标开始寻找|                  
                start=end+1;

        }

	chatdb->my_database_disconnect();
}

//添加好友
void Server::server_add_friend(struct bufferevent *bev,Json::Value val)
{
	chatdb->my_database_connect("user");
	Json::Value v;
	string s;
	//如果好友不存在
	if(!chatdb->my_database_user_exist(val["friend"].asString()))
	{	
	        v["cmd"]="add_reply";
	        v["result"]="user_not_exist";

        	//发送给客户端
       	        s=Json::FastWriter().write(v);
		if(bufferevent_write(bev,s.c_str(),strlen(s.c_str()))<0)
                {
               		cout<<"bufferevent write"<<endl;
                }
		return;
	}
	
	//如果是好友关系
	if(chatdb->my_database_is_friend(val["user"].asString(),val["friend"].asString()))
	{	
		v.clear();
		v["cmd"]="add_reply";
                v["result"]="already_friend";

                //发送给客户端
                s=Json::FastWriter().write(v);
                if(bufferevent_write(bev,s.c_str(),strlen(s.c_str()))<0)
                {
                        cout<<"bufferevent write"<<endl;
                }
		return;
	}else{

	//修改双方数据库
	chatdb->my_database_add_new_friend(val["user"].asString(),val["friend"].asString());
	chatdb->my_database_add_new_friend(val["friend"].asString(),val["user"].asString());

	//返回自己的客户端添加成功
	v.clear();
        v["cmd"]="add_reply";
        v["result"]="success";
	v["friend"]=val["friend"];

        s=Json::FastWriter().write(v);
        if(bufferevent_write(bev,s.c_str(),strlen(s.c_str()))<0)
        {
               cout<<"bufferevent write"<<endl;
        }
	
	//返回对方的客户端有好友添加（先查看是否在线）
	for(list<User>::iterator it=chatlist->online_user->begin();it!=chatlist->online_user->end();it++)
        {
		//如果好友在线
                if(val["friend"]==it->name)
                {
                         v.clear();
                         v["cmd"]="add_friend_reply";
                         v["result"]=val["user"];

                         //发送给客户端
                         s=Json::FastWriter().write(v);
                         if(bufferevent_write(it->bev,s.c_str(),strlen(s.c_str()))<0)
                         {
                                 cout<<"bufferevent write"<<endl;
                         }

                }
        }
	
	}
	
	chatdb->my_database_disconnect();
}

//建群
void Server::server_create_group(struct bufferevent *bev,Json::Value val)
{
	chatdb->my_database_connect("chatgroup");
	
	//判断群是否存在
       	if(chatdb->my_database_group_exist(val["group"].asString()))
	{
		Json::Value v;
                v["cmd"]="create_group_reply";
                v["result"]="group_exist";

                //发送给客户端
                string s=Json::FastWriter().write(v);
                if(bufferevent_write(bev,s.c_str(),strlen(s.c_str()))<0)
                {
                        cout<<"bufferevent write"<<endl;
                }
		return;
	}

	//操作数据库添加群
	chatdb->mydatabase_add_new_group(val["group"].asString(),val["user"].asString());
	chatdb->my_database_disconnect();

	chatdb->my_database_connect("user");
	//修改数据库个人信息
	chatdb->my_database_user_add_group(val["user"].asString(),val["group"].asString());
	
	//修改群链表
	chatlist->info_add_new_group(val["group"].asString(),val["user"].asString());

        Json::Value val1;
        val1["cmd"]="create_group_reply";
        val1["result"]="success";
	val1["group"]=val["group"];

        string s=Json::FastWriter().write(val1);
        if(bufferevent_write(bev,s.c_str(),strlen(s.c_str()))<0)
        {
               cout<<"bufferevent write"<<endl;
        }

	chatdb->my_database_disconnect();
}

//添加群聊
void Server::server_add_group(struct bufferevent *bev,Json::Value val)
{
	//判断群是否存在
	if(!chatlist->info_group_exist(val["group"].asString()))
	{
		Json::Value val1;
	        val1["cmd"]="add_group_reply";
       	        val1["result"]="group_not_exist";

   	        string s=Json::FastWriter().write(val1);
 	        if(bufferevent_write(bev,s.c_str(),strlen(s.c_str()))<0)
	        {
	               cout<<"bufferevent write"<<endl;
	        }
	return;	
	}
	//判断用户是否在群里
	if(chatlist->info_user_in_group(val["group"].asString(),val["user"].asString()))
	{
		Json::Value val1;
                val1["cmd"]="add_group_reply";
                val1["result"]="user_in_group";

                string s=Json::FastWriter().write(val1);
                if(bufferevent_write(bev,s.c_str(),strlen(s.c_str()))<0)
                {
                       cout<<"bufferevent write"<<endl;
                }
	return;
	}

	//修改数据库（用户表、群表）
	chatdb->my_database_connect("user");
	chatdb->my_database_user_add_group(val["user"].asString(),val["group"].asString());
	chatdb->my_database_disconnect();

	chatdb->my_database_connect("chatgroup");
        chatdb->my_database_group_add_user(val["group"].asString(),val["user"].asString());
        chatdb->my_database_disconnect();

	//修改链表
	chatlist->info_group_add_user(val["group"].asString(),val["user"].asString());
	
	Json::Value val1;
        val1["cmd"]="add_group_reply";
        val1["result"]="success";
	val1["group"]=val["group"];


        string s=Json::FastWriter().write(val1);
        if(bufferevent_write(bev,s.c_str(),strlen(s.c_str()))<0)
        {
               cout<<"bufferevent write"<<endl;
        }
}

//私聊
void Server::server_private_chat(struct bufferevent *bev,Json::Value val)
{
	//to_bev是要传的那个人的bev
	struct bufferevent *to_bev=chatlist->info_get_friend_bev(val["user_to"].asString());
	//当前用户不在线
	if(NULL==to_bev)
	{
		Json::Value val1;
	        val1["cmd"]="private_chat_reply";
	        val1["result"]="offline";

	        string s=Json::FastWriter().write(val1);
	        if(bufferevent_write(bev,s.c_str(),strlen(s.c_str()))<0)
	        {
	               cout<<"bufferevent write"<<endl;
	        }

	}
	//把数据发过去(原封不动的转发过去,对方为to_bev)
	string s=Json::FastWriter().write(val);
        if(bufferevent_write(to_bev,s.c_str(),strlen(s.c_str()))<0)
        {
               cout<<"bufferevent write"<<endl;
        }
	
	//回复客户端发送成功	
	Json::Value val1;
        val1["cmd"]="private_chat_reply";
        val1["result"]="success";

        s=Json::FastWriter().write(val1);
        if(bufferevent_write(bev,s.c_str(),strlen(s.c_str()))<0)
        {
               cout<<"bufferevent write"<<endl;
        }
}

//群聊
void Server::server_group_chat(struct bufferevent *bev,Json::Value val)
{
	for(list<Group>::iterator it=chatlist->group_info->begin();it!=chatlist->group_info->end();it++)
	{
		//找到group，遍历group中的user
		if(val["group"].asString()==it->name)
		{
			for(list<GroupUser>::iterator i=it->l->begin();i!=it->l->end();i++)
			{
				struct bufferevent* to_bev=chatlist->info_get_friend_bev(i->name);
			
				cout<<"sbsb"<<endl;

				//如果在线，转发过去
				if(to_bev!=NULL)
				{
		
					cout<<"sbsbs"<<endl;
	
					string s=Json::FastWriter().write(val);
				        if(bufferevent_write(to_bev,s.c_str(),strlen(s.c_str()))<0)
        				{
            					cout<<"bufferevent write"<<endl;
       					}

				}
			}
		}
	}
	//回复客户端发送成功    
        Json::Value val1;
        val1["cmd"]="group_chat_reply";
        val1["result"]="success";

        string s=Json::FastWriter().write(val1);
        if(bufferevent_write(bev,s.c_str(),strlen(s.c_str()))<0)
        {
               cout<<"bufferevent write"<<endl;
        }
}
	
void Server::server_get_group_member(struct bufferevent *bev,Json::Value val)
{
	string member=chatlist->info_get_group_member(val["group"].asString());

	Json::Value val1;
        val1["cmd"]="get_group_member_reply";
        val1["result"]=member;
	val1["group"]=val["group"];

        string s=Json::FastWriter().write(val1);
        if(bufferevent_write(bev,s.c_str(),strlen(s.c_str()))<0)
        {
               cout<<"bufferevent write"<<endl;
        }
}

void Server::server_user_offline(struct bufferevent *bev,Json::Value val)
{
	//从链表中删除用户
        User u={val["user"].asString(),bev};
        
	for(list<User>::iterator it=chatlist->online_user->begin();it!=chatlist->online_user->end();it++)
	{
		if(it->name==val["user"].asString())
		{
			chatlist->online_user->erase(it);
			break;
		}
	}	
	

        //获取好友列表以及群列表
        string friend_list,group_list;
	string name,s;
	Json::Value v;
	chatdb->my_database_connect("user");
        chatdb->my_database_get_friend_group(val["user"].asString(),friend_list,group_list);

        //好友上线提醒
        int start =0;    //字符串下标
        int end = 0;    //字符串下标
	int flag=1;
	while(flag)
        {
                //find(要查找的字符串，从start下标开始查找) 返回| 的下标
                end=friend_list.find("|",start);

                //结束跳出循环
                if(end ==-1)
                {
			name=friend_list.substr(start,friend_list.size()-start);
                        flag=0;
                }
		else
		{
                	//substr(从什么时候开始读，读几个)end-start 表示2-0=2，读两个字，返回读到的字符串
                	name=friend_list.substr(start,end - start);
		}
                //遍历链表                
                for(list<User>::iterator it=chatlist->online_user->begin();it!=chatlist->online_user->end();it++)
                {
                        if(name==it->name)
                        {
                                v.clear();
                                v["cmd"]="friend_online";
                                v["friend"]=val["user"];
                                //发送给客户端
                                s=Json::FastWriter().write(v);
                                if(bufferevent_write(it->bev,s.c_str(),strlen(s.c_str()))<0)
				{
                                        cout<<"bufferevent write"<<endl;
                                }
                        }
                }
                //下一次从end+1的坐标开始寻找|                  
                start=end+1;
        }
	chatdb->my_database_disconnect();
}


void Server::send_file_handler(int length,int port,int* f_fd,int*t_fd)
{
	int sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(-1==sockfd)
	{
		return;
	}
	
	struct sockaddr_in server_addr,client_addr;
	memset(&server_addr,0,sizeof(server_addr));
    	server_addr.sin_family=AF_INET;
    	server_addr.sin_port=htons(port);
    	server_addr.sin_addr.s_addr=inet_addr(IP);

	int len=sizeof(client_addr);
	//接受发送客户端的连接请求    
	*f_fd=accept(sockfd,(struct sockaddr *)&client_addr,(socklen_t*)&len);   
	
	//接受接收客户端的连接请求
	*t_fd=accept(sockfd,(struct sockaddr *)&client_addr,(socklen_t*)&len);

	char buf[4096]={0};
	size_t size,sum=0;
	while(1)
	{
		size=recv(*f_fd,buf,4096,0);
		sum+=size;
		send(*t_fd,buf,size,0);
		if(sum>=length)
		{
			break;
		}
		memset(buf,0,4096);
	}	
	

}


void Server::server_send_file(struct bufferevent *bev,Json::Value val)
{
	Json::Value v;
	string s;

	struct bufferevent* to_bev=chatlist->info_get_friend_bev(val["to_user"].asString());
	if(to_bev==NULL)//对方不在线
	{
	        v["cmd"]="send_file_reply";
        	v["result"]="offline";

	        s=Json::FastWriter().write(v);
        	if(bufferevent_write(bev,s.c_str(),strlen(s.c_str()))<0)
	        {
        	       cout<<"bufferevent write"<<endl;
	        }
		return;
	}
	//启动新线程，创建文件服务器
	int port=8080,from_fd=0,to_fd=0;
	thread send_file_thread(send_file_handler,val["length"].asInt(),port,&from_fd,&to_fd);
	send_file_thread.detach();

	v.clear();
	v["cmd"]="send_file_port_reply";
        v["result"]=port;

        s=Json::FastWriter().write(v);
        if(bufferevent_write(bev,s.c_str(),strlen(s.c_str()))<0)
        {
               cout<<"bufferevent write"<<endl;
        }

	int count=0;
	while(from_fd<=0)
	{	
		//超过十秒未响应，取消线程
		count++;
		usleep(100000);
		if(count==100)
		{
			pthread_cancel(send_file_thread.native_handle());//取消线程
			v.clear();
		        v["cmd"]="send_file_reply";
		        v["result"]="timeout";

		        s=Json::FastWriter().write(v);
		        if(bufferevent_write(bev,s.c_str(),strlen(s.c_str()))<0)
		        {
		               cout<<"bufferevent write"<<endl;
		        }
			return;
		}
	}
	
	//返回端口号给接受客户端
	v.clear();
        v["cmd"]="recv_file_port_reply";
        v["result"]=port;

        s=Json::FastWriter().write(v);
        if(bufferevent_write(to_bev,s.c_str(),strlen(s.c_str()))<0)
        {
               cout<<"bufferevent write"<<endl;
        }

	count=0;
        while(to_fd<=0)
        {
                //超过十秒未响应，取消线程
                count++;
                usleep(100000);
                if(count==100)
                {
                        pthread_cancel(send_file_thread.native_handle());//取消线程
                        v.clear();
                        v["cmd"]="send_file_reply";
                        v["result"]="timeout";

                        s=Json::FastWriter().write(v);
                        if(bufferevent_write(bev,s.c_str(),strlen(s.c_str()))<0)
                        {
                               cout<<"bufferevent write"<<endl;
                        }
                        return;
                }
        }
}	






