#include"chat_database.h"

ChatDatabase::ChatDatabase()
{
	
}

ChatDatabase::~ChatDatabase()
{
	mysql_close(mysql);
}

void ChatDatabase::my_database_connect(const char *name)
{
	mysql=mysql_init(NULL);
	
	mysql=mysql_real_connect(mysql,"localhost","root","123456",name,0,NULL,0);//name:数据库名
	if(NULL==mysql)
	{
		cout<<"connect database failure" <<endl;
	}
	
	//设置中文编码	
	if(mysql_query(mysql,"set names utf8;")!=0)
        {
                cout<<"mysql_query error"<<endl;
        }

}

int ChatDatabase::my_database_get_group_name(string *s)
{
	if(mysql_query(mysql,"show tables;")!=0)
	{
		cout<<"mysql_query error"<<endl;
	}

	//获取数据集（表）
	MYSQL_RES *res = mysql_store_result(mysql);

	if(NULL == res)
	{
		cout<<"mysql_store_result error"<<endl;
	}

	//获取多行数据集
	MYSQL_ROW row;

	int count=0;
	while(row = mysql_fetch_row(res))
	{
		//把表名一行一行输入到数组中
		s[count] += row[0];
		count++;
	}
	return count;
}



void ChatDatabase::my_database_get_group_member(string name,string &s)
{	//数据库语句
	char sql[1024]={0};
	
	//把传进来的表面接上去然后赋值给sql
	sprintf(sql,"select member from %s;",name.c_str());

	//执行一条sql语句，赋值给mysql结构体，若失败则报错
	if(mysql_query(mysql,sql)!=0)
	{
		cout<<"mysql_query error"<<endl;
	}

	//获取结果集
	MYSQL_RES *res=mysql_store_result(mysql);
        if(NULL ==res)
	{
		cout<<"mysql_store_result error"<<endl;
	}

	//获取一行结果
	MYSQL_ROW row = mysql_fetch_row(res);

	if(row[0]==NULL)
	{
		return;
	}	

	//把结果赋值给string &s
	s+=row[0];

}

bool ChatDatabase::my_database_user_exist(string name)
{
	char sql[128]={0};
	sprintf(sql,"show tables like '%s';",name.c_str());
	
	if(mysql_query(mysql,sql)!=0)
        {
                cout<<"mysql_query error"<<endl;
        }
	
	MYSQL_RES *res=mysql_store_result(mysql);
	MYSQL_ROW row=mysql_fetch_row(res);
	if(row==NULL)
	{
		//用户不存在
		return false;
	}
	else
	{	
		//用户存在
		return true;
	}
}


void ChatDatabase::my_database_user_password(string name,string password)
{
	char sql[128]={0};
	sprintf(sql,"create table %s(password varchar(16),friend varchar(4096),chatgroup varchar(4096)) character set utf8;",name.c_str());
	if (mysql_query(mysql,sql)!=0)
	{
		cout<<"mysql_query error"<<endl;
	}

	//清空sql,添加密码
	memset(sql,0,sizeof(sql));
	sprintf(sql,"insert into %s (password) values ('%s');",name.c_str(),password.c_str());
	if(mysql_query(mysql,sql)!=0)
	{
		cout<<"mysql_query error";
	}
}

bool ChatDatabase::my_database_password_correct(string name,string password)
{	
	char sql[128]={0};
        sprintf(sql,"select password from %s",name.c_str());
        if (mysql_query(mysql,sql)!=0)
        {
                cout<<"mysql_query error"<<endl;
        }

	//获取结果集
        MYSQL_RES *res=mysql_store_result(mysql);
        if(NULL ==res)
        {
                cout<<"mysql_store_result error"<<endl;
        }

        //获取一行结果
        MYSQL_ROW row = mysql_fetch_row(res);
	
	if(row[0]==password)//密码正确
	{
		return true;
	}
	else//密码错误
	{
		return false;
	}
}

void ChatDatabase::my_database_get_friend_group(string name,string &f,string &g)
{
	char sql[128]={0};
        sprintf(sql,"select friend from %s;",name.c_str());
        if (mysql_query(mysql,sql)!=0)
        {
                cout<<"mysql_query error"<<endl;
        }
        //获取结果集
        MYSQL_RES *res=mysql_store_result(mysql);
        //获取一行结果
        MYSQL_ROW row = mysql_fetch_row(res);
        if(row[0]!=NULL)//好友不为空,获取好友列表
        {
                f.append(row[0]);
        }
	mysql_free_result(res);

	memset(sql,0,sizeof(sql));
        sprintf(sql,"select chatgroup from %s;",name.c_str());
        if (mysql_query(mysql,sql)!=0)
        {
                cout<<"mysql_query error"<<endl;
        }
        //获取结果集
        res=mysql_store_result(mysql);
        //获取一行结果
        row = mysql_fetch_row(res);
        if(row[0]!=NULL)//好友不为空,获取好友列表
        {
                g.append(row[0]);
        }
   

}

bool ChatDatabase::my_database_is_friend(string n1,string n2)
{
	char sql[128]={0};
	sprintf(sql,"select friend from %s;",n1.c_str());
	if(mysql_query(mysql,sql)!=0)
	{
		cout<<"mysql_query error"<<endl;
	}	

	MYSQL_RES *res = mysql_store_result(mysql);
	MYSQL_ROW row =mysql_fetch_row(res);
	if(NULL==row[0])
	{
		//没有好友
		return false;
	}
	else
	{	//有好友
		string all_friend(row[0]);
		int start =0,end = 0;
		while(1)
		{
			end = all_friend.find('|',start );
			if(-1==end)
			{
				break;
			} 
			if(n2==all_friend.substr(start,end-start))
			{
				return true;
			}
			start=end+1;
		}

		if(n2==all_friend.substr(start,all_friend.size()-start))
                {
                        return true;
                }
		
		return false;
						
	}
}

void ChatDatabase::my_database_add_new_friend(string n1,string n2)
{
	char sql[1024]={0};
	//先把原来的friend读出来
	sprintf(sql,"select friend from %s",n1.c_str());
	if(mysql_query(mysql,sql)!=0)
        {
                cout<<"mysql_query error"<<endl;
        }
	//好友列表
	string friend_list;
	MYSQL_RES *res = mysql_store_result(mysql);
        MYSQL_ROW row =mysql_fetch_row(res);
	//如果好友列表为空
	if(row[0]==NULL)
	{
		friend_list.append(n2);
	}
	else
	{	
		//拼接friend
		friend_list.append(row[0]);
		friend_list+="|";
		friend_list+=n2;
	}
	
	sprintf(sql,"update %s set friend = '%s';",n1.c_str(),friend_list.c_str());
        if(mysql_query(mysql,sql)!=0)
        {
                cout<<"mysql_query error"<<endl;
        }
	
}

bool ChatDatabase::my_database_group_exist(string group_name)
{
        char sql[1024]={0};
        sprintf(sql,"show tables like '%s';",group_name.c_str());

        if(mysql_query(mysql,sql)!=0)
        {
                cout<<"mysql_query error"<<endl;
        }

        MYSQL_RES *res=mysql_store_result(mysql);
        MYSQL_ROW row=mysql_fetch_row(res);
        if(row==NULL)
        {
                //群不存在
                return false;
        }
        else
        {
                //群存在
                return true;
        }

}

void ChatDatabase::mydatabase_add_new_group(string group_name,string owner)
{
	char sql[128]={0};
	sprintf(sql,"create table %s (owner varchar(32),member varchar(4096)) character set utf8;",group_name.c_str());
	if(mysql_query(mysql,sql)!=0)
        {
                cout<<"mysql_query error"<<endl;
        }
	
	memset(sql,0,sizeof(sql));
        sprintf(sql,"insert into %s values ('%s','%s');",group_name.c_str(),owner.c_str(),owner.c_str());
        if(mysql_query(mysql,sql)!=0)
        {
                cout<<"mysql_query error"<<endl;
        }

}

//加入群聊
void  ChatDatabase::my_database_user_add_group(string user_name,string group_name)
{
	char sql[1024]={0};
        sprintf(sql,"select chatgroup from %s;",user_name.c_str());

        if(mysql_query(mysql,sql)!=0)
        {
                cout<<"mysql_query error"<<endl;
        }

	string all_group;
        MYSQL_RES *res=mysql_store_result(mysql);
        MYSQL_ROW row=mysql_fetch_row(res);
        if(row[0]!=NULL)
        {
		all_group+=row[0];
		all_group+="|";
		all_group+=group_name;
        }
        else
        {
               all_group+=group_name;
        }

	memset(sql,0,sizeof(sql));
	sprintf(sql,"update %s set chatgroup='%s';",user_name.c_str(),all_group.c_str());
	if(mysql_query(mysql,sql)!=0)
        {
                cout<<"mysql_query error"<<endl;
        }


}


void ChatDatabase::my_database_group_add_user(string group_name,string user_name)
{
	char sql[1024]={0};
        sprintf(sql,"select member from %s;",group_name.c_str());

        if(mysql_query(mysql,sql)!=0)
        {
                cout<<"mysql_query error"<<endl;
        }

        string all_member;
        MYSQL_RES *res=mysql_store_result(mysql);
        MYSQL_ROW row=mysql_fetch_row(res);
        if(row[0]!=NULL)
        {
                all_member+=row[0];
                all_member+="|";
                all_member+=user_name;
        }
        else
        {
               all_member+=user_name;
        }

        memset(sql,0,sizeof(sql));
        sprintf(sql,"update %s set member='%s';",group_name.c_str(),all_member.c_str());
        if(mysql_query(mysql,sql)!=0)
        {
	        cout<<"mysql_query error"<<endl;
        }

}


void ChatDatabase::my_database_disconnect()
{
	mysql_close(mysql);
}












