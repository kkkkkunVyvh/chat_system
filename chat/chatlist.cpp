#include"chatlist.h"

ChatInfo::ChatInfo()
{
	//创建在线用户链表
	online_user=new list<User>;
	
	//创建所有群的链表
	group_info=new list<Group>;

	//往group_info链表中添加群信息
	mydatabase = new ChatDatabase;

	//发起连接
	mydatabase->my_database_connect("chatgroup");

	//获取群聊个数
	string group_name[MAXNUM];
	int group_num = mydatabase->my_database_get_group_name(group_name);	 

	//往群链表里面添加Group结构体
	for(int i=0;i<group_num;i++)
	{
		//创建群的结构体
		Group g;
		
		//拿到群昵称，赋值给每个群结构体
		g.name=group_name[i];

		//g.l对应群成员的链表
		g.l=new list<GroupUser>;
	
		//把group放入grouplist中
                group_info->push_back(g);
	
		//保存群里面所有用户
		string member;
		mydatabase->my_database_get_group_member(group_name[i],member);	
	
		if(member.size()==0)
		{
			continue;
		}
		
	        //操作字符串，把每个人的名字单独拿出来放进g.l中                  
		int start =0;    //字符串下标
		int end = 0;    //字符串下标
		GroupUser u;
		while(1) 
		{
			//find(要查找的字符串，从start下标开始查找) 返回| 的下标
			end=member.find('|',start);

			//结束跳出循环
			if(end ==-1)
			{
 				break;
			}

			//substr(从什么时候开始读，读几个)end-start 表示2-0=2，读两个字，返回读到的字符串
			u.name=member.substr(start,end - start);
		
			g.l->push_back(u);
			//下一次从end+1的坐标开始寻找|			
		
			start=end+1;
			
			//每次用完把u清除
			u.name.clear();
		}

		//把最后一个成员放入
		u.name=member.substr(start,member.size() - start);
		g.l->push_back(u);

	}

	/* //打印每个群的群成员
        for(list<Group>::iterator it=group_info->begin();it!=group_info->end();it++)
        {
                       	cout<<"群名："<<it->name<<endl;
			for(list<GroupUser>::iterator i=it->l->begin();i!=it->l->end();i++)
			{
				cout<<i->name<<"  ";
			}
			cout<<endl;				
        }
	*/

	//关闭数据库
	mydatabase->my_database_disconnect();

	cout<<"初始化链表成功"<<endl;

	
}

//判断群是否存在
bool ChatInfo::info_group_exist(string group_name)
{
	for(list<Group>::iterator it=group_info->begin();it!=group_info->end();it++)
	{
		if(it->name==group_name)
		{
			return true;
		}
	}	
	return false;
}

//判断用户是否在群里
bool ChatInfo::info_user_in_group(string group_name,string user_name)
{
	for(list<Group>::iterator it=group_info->begin();it!=group_info->end();it++)
        {
                if(it->name==group_name)
                {
                        for(list<GroupUser>::iterator i=it->l->begin();i!=it->l->end();i++)
			{
				if(i->name==user_name)
				{
					return true;
				}
			}
                }
        }
        return false;

}
 
////在群链表中的list<GroupUser>中添加GroupUser
void ChatInfo::info_group_add_user(string group_name,string user_name) 
{
	 for(list<Group>::iterator it=group_info->begin();it!=group_info->end();it++)
        {
                if(it->name==group_name)
                {
                        GroupUser u;
			u.name=user_name;
			it->l->push_back(u);
                }
        }

}

struct bufferevent* ChatInfo::info_get_friend_bev(string name)
{
	for(list<User>::iterator it=online_user->begin();it!=online_user->end();it++)
	{
		if(it->name==name)
		{
			return it->bev;
		}
	}
	return NULL;
}

string ChatInfo::info_get_group_member(string group_name)
{	
	string member;
	for(list<Group>::iterator it=group_info->begin();it!=group_info->end();it++)
        {
                if(it->name==group_name)
                {
                        for(list<GroupUser>::iterator i=it->l->begin();i!=it->l->end();i++)
                        {
				member+=i->name;
				member+="|";
                        }
                }
        }
	return member;

}

void ChatInfo::info_add_new_group(string group_name,string user_name)
{
	Group g;
	g.name=group_name;
	g.l=new list<GroupUser>;
	group_info->push_back(g);

	GroupUser u;
	u.name=user_name;
	g.l->push_back(u);
}


