#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>


#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include "poolsocket.h"
#include "keymngserverop.h"


MngServer_Info   		mngServerInfo;
int						g_EndTag  = 0;;


//业务线程

//1 收客户端的请求报文 
//2 解析客户端的请求报文
//3 cmdType 123
//4 组织应答报文 编码应答报文
//5 回发应答报文

void *mystart_routine (void *arg)
{
	
	int 					ret = 0;
	int 					connfd  = (int)arg;
	int 					mytimeout = 3;
	
	//接受的客户端的请求报文 编码以后的结果
	unsigned char 			*out = NULL;
	int 					outlen = 0;
		
		
	//客户端请求报文数据结构
	MsgKey_Req				*pMsgKeyReq = NULL;
	int 					iMsgKeyReq = 0;
	
	//服务器应答报文 编码以后的结果
	unsigned char 			*pMsgKey_ResData = NULL;
	int 					iMsgKey_ResDataLen = 0;
	
	
	while (1)
	{
		if (g_EndTag ==1)
		{
			break;
		}
		out = NULL;
		outlen = 0;
		
		//客户端请求报文数据结构
		pMsgKeyReq = NULL;
		iMsgKeyReq = 0;
		
		//服务器应答报文 编码以后的结果
		pMsgKey_ResData = NULL;
		iMsgKey_ResDataLen = 0;
	
		
		//服务器端端接受报文
		//异常情况 1对方已经关闭 2 出错 3 超时
		ret =  sckServer_rev(connfd, mytimeout, &out, &outlen); //1 sck_FreeMem((void ** )&out);
		if (ret == Sck_ErrPeerClosed)
		{
			printf("服务端监测出 客户端已经关闭\n");
			break;	
		}
		else if (ret == Sck_ErrTimeOut)
		{
			goto End;	
		}
		else if (ret != 0)
		{
			printf("func sckServer_rev() err:%d \n", ret);
			break;
		}
	

		//2 解析客户端的请求报文
		ret = MsgDecode(out, outlen, (void **)&pMsgKeyReq, &iMsgKeyReq); //2  MsgMemFree((void **) &pMsgKeyReq, iMsgKeyReq);
		if (ret != 0)
		{
			printf("func MsgDecode() err:%d \n", ret);
			goto End;
		}

		//3 cmdType 123
		switch (pMsgKeyReq->cmdType)
		{
		case KeyMng_NEWorUPDATE:
			//密钥协商应答
			ret = MngServer_Agree(&mngServerInfo, pMsgKeyReq, &pMsgKey_ResData, &iMsgKey_ResDataLen); //3 MsgMemFree( (void **) &pMsgKey_ResData, 0);
			break;
		case KeyMng_Check:
			//密钥校验
			ret = MngServer_Check(&mngServerInfo, pMsgKeyReq, &pMsgKey_ResData, &iMsgKey_ResDataLen); 
			break;
		case KeyMng_Revoke:
			//密钥注销
			break;	
		default:
			ret = MngSvr_ParamErr;
			break;		
			
		}
		if  (ret != 0)
		{
			printf("服务器端业务流处理失败 err:%d .\n",ret);
			//MsgKey_Res			msgKeyRes;
			//msgKeyRes
			//msgKeyRes.rv = 200;
			//sckServer_send(connfd, mytimeout, pMsgKey_ResData, iMsgKey_ResDataLen);
			goto End;	
		}
	
		//服务器端发送报文
		ret = sckServer_send(connfd, mytimeout, pMsgKey_ResData, iMsgKey_ResDataLen);
		if (ret == Sck_ErrPeerClosed)
		{
			printf("服务端监测出 客户端已经关闭\n");
			break;	
		}
		else if (ret == Sck_ErrTimeOut)
		{
			goto End;	
		}
		else if (ret != 0)
		{
			sck_FreeMem((void ** )&out);
			printf("func sckServer_rev() err:%d \n", ret);
			break;
		}
	
	End:
			if (out) 				sck_FreeMem((void ** )&out);
			if (pMsgKeyReq)			MsgMemFree((void **) &pMsgKeyReq, iMsgKeyReq);
			if (pMsgKey_ResData)	MsgMemFree( (void **) &pMsgKey_ResData, 0);
	}		
	
	if (out) 				sck_FreeMem((void ** )&out);
	if (pMsgKeyReq)			MsgMemFree((void **) &pMsgKeyReq, iMsgKeyReq);
	if (pMsgKey_ResData)	MsgMemFree( (void **) &pMsgKey_ResData, 0);

	
	
	sckServer_close(connfd);
	
	
	return NULL;	
}

int mainx()
{
	fork();
	
	fork();
	
	fork();
	
	printf("aaaaaa\n");	
}


#define INIT_DAEMON \
{ \
	if(fork() >0) exit(0); \
	setsid(); \
	if(fork()>0) exit(0); \
}

/*
#define INIT_DAEMON \
{ 
	if(fork() >0)
	{
		exit(0); 
	} 
	setsid(); 
	
	if(fork()>0) 
	{
		exit(0); 	
	}	
}*/


/* 
typedef void (*sighandler_t)(int);
sighandler_t signal(int signum, sighandler_t handler);
*/

//sighandler_t  mysingal;
//int signal(int signum,   void (*sighandler_t)(int arg));

//函数指针做函数参数  回调函数
//1 语法上： 函数指针做函数参数 
//2 使用上：谁调用函数指针做函数参数的 api ， 谁提供被调用函数的入口地址
//3 回调的效果图 为什么说是回调？
	//本来你调用linux 结果linux反过来又调用上层应用的api
//4 为什么用回调函数 回调函数的意义  起到什么作用呢？
	//把任务的调用者 和 任务的编写者进行有效的解耦合
	

void mysighandler_t(int arg)
 {
 	printf("func mysighandler_t() begin arg:%d\n", arg);
 	g_EndTag = 1;
 	
 	printf("func mysighandler_t() end\n");
 	
 	return ;
 }
 
 
int main()
{
	int 		ret = 0;
	int 		listenfd = 0;
	
	int 		connfd = 0;
	int 		mytime = 3;
	pthread_t	pid;
	
	printf("keymngserver hello...\n");
	
	INIT_DAEMON


	//安装一个信号处理函数
	signal(SIGUSR1, mysighandler_t); //告诉linux内核 当有人向我发送用户自定义信号1的时候 请你调用myfunc api函数
	signal(SIGPIPE, SIG_IGN); //告诉linux内核 当有管道破裂时候 ，请你不要在发送SIGPIPE 让1.exe死掉 请你忽略
	
	memset(&mngServerInfo, 0, sizeof(MngServer_Info));
	
	
	//初始化服务器 全局变量
	ret = MngServer_InitInfo(&mngServerInfo);
	if (ret != 0)
	{
		printf("func MngServer_InitInfo() err:%d \n", ret);
		return ret;
	}
	
	//函数声明
	//服务器端初始化
	ret =  sckServer_init(mngServerInfo.serverport, &listenfd);
	if (ret != 0)
	{
		printf("func sckServer_init() err:%d \n", ret);
		return ret;	
	}
	
	while (1)
	{
		if (g_EndTag == 1)
		{
			break;
		}
		ret = sckServer_accept(listenfd, mytime, &connfd);
		if (ret == Sck_ErrTimeOut)
		{
			printf("func sckServer_accept() 检测到内核中没有连接 \n ");
			continue;
		}
		
       pthread_create(&pid,  NULL, mystart_routine, (void *)connfd);

	}
	
	
	sleep(2);
	
	
	//释放资源
	sckClient_closeconn(listenfd);
	//服务器端环境释放 
	sckServer_destroy();
	
	//释放数据库连接池
	IC_DBApi_PoolFree();

	printf("keymngserver优雅退出\n");
	return 0;	
}

