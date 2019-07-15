#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <pthread.h>

#include "keymnglog.h"
#include "keymngserverop.h"
#include "poolsocket.h"
#include "keymng_msg.h"
#include "myipc_shm.h"

#include "keymng_shmop.h"
#include "icdbapi.h"
#include "keymng_dbop.h"

//读配置文件获取数据库的连接信息 user pw sid
//从数据库中 获取服务器的配置参数

int MngServer_InitInfo(MngServer_Info *svrInfo)
{
	int 			ret  = 0;
	//读配置信息 获取  user pw sid
	
	strcpy(svrInfo->serverId, "0001");
	strcpy(svrInfo->dbuse, "SECMNG");
	strcpy(svrInfo->dbpasswd, "SECMNG");
	strcpy(svrInfo->dbsid, "orcl");
	svrInfo->dbpoolnum = 5;  //数据库连接池的个数
	
	strcpy(svrInfo->serverip, "127.0.0.1");
	svrInfo->serverport = 8001;
	
	svrInfo->maxnode = 20;
	svrInfo->shmkey = 0x0001;
	
	//初始化共享内存
	ret = KeyMng_ShmInit(svrInfo->shmkey, svrInfo->maxnode, &svrInfo->shmhdl);
	if (ret != 0)
	{
		printf("func KeyMng_ShmInit() err:%d \n", ret);
		return ret;
	}
	
	//初始化数据库连接池
	ret = IC_DBApi_PoolInit(svrInfo->dbpoolnum , svrInfo->dbsid, svrInfo->dbuse, svrInfo->dbpasswd);
	if (ret != 0)
	{
		printf("func IC_DBApi_PoolInit() err:%d \n", ret);
		return ret;
	}
	printf("系统初始化 数据库连接池 ok\n");
	
	
	return ret;	
}

//static int g_keynum =  101;
//服务器端 应答流程
int MngServer_Agree(MngServer_Info *svrInfo, MsgKey_Req *msgkeyReq, unsigned char **outData, int *datalen)
{
	int 				i = 0, ret = 0;
	//组织 应答报文
	MsgKey_Res			msgKeyRes;
	ICDBHandle			handle = NULL;
	int 				tmpkeyid = 0;
	
	KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[1], ret,"func MngServer_Agree() Begin ");
	
	memset(&msgKeyRes, 0, sizeof(MsgKey_Res) );
	
	msgKeyRes.rv = 0;
	strcpy(msgKeyRes.clientId, msgkeyReq->clientId);
	strcpy(msgKeyRes.serverId, svrInfo->serverId);
	
	
	//获取连接
	ret =  IC_DBApi_ConnGet(&handle, 0, 0);
	if (ret != 0)
	{
		KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[4], ret,"func IC_DBApi_ConnGet() err ");
		return ret;
	}
	
	//开事物
	ret = IC_DBApi_BeginTran(handle);
	if (ret != 0)
	{
		KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[4], ret,"func IC_DBApi_BeginTran() err ");
		goto End;
	}
	
	//从数据库中获取密钥序列号
	//8  --->8   9
	ret =  KeyMngsvr_DBOp_GenKeyID(handle, &tmpkeyid);
	if (ret != 0)
	{
		KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[4], ret,"func KeyMngsvr_DBOp_GenKeyID() err ");
		goto End;
	}
	
	msgKeyRes.seckeyid = tmpkeyid; //赋给应答报文
	for (i=0; i<64; i++)
	{
		msgKeyRes.r2[i] = 'a' + i;
	}
	
	//编码应答报文
	ret = MsgEncode(&msgKeyRes, ID_MsgKey_Res, outData, datalen);
	if (ret != 0)
	{
		KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[4], ret,"func MsgEncode() err ");
		goto End;
	}
	
	//协商密钥
	NodeSHMInfo  nodeSHMInfo;
	memset(&nodeSHMInfo, 0, sizeof(NodeSHMInfo));
	nodeSHMInfo.status = 0;
	strcpy(nodeSHMInfo.clientId, msgkeyReq->clientId);
	strcpy(nodeSHMInfo.serverId, msgkeyReq->serverId);
	nodeSHMInfo.seckeyid = msgKeyRes.seckeyid;
	
	//r1{abc} r2{123}
	//a1b2c3d4
	for (i=0; i<64; i++)
	{
		nodeSHMInfo.seckey[2*i] = msgkeyReq->r1[i];
		nodeSHMInfo.seckey[2*i + 1] = msgKeyRes.r2[i];
	}

	//写服务器的共享内存
	ret = KeyMng_ShmWrite(svrInfo->shmhdl, svrInfo->maxnode, &nodeSHMInfo);
	if (ret != 0)
	{
		KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[1], ret,"func KeyMng_ShmWrite() err:%d \n", ret);
		goto End;
	}
	
	//写数据库
	KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[1], ret,"00000000000000000000000000");
	ret = KeyMngsvr_DBOp_WriteSecKey(handle, &nodeSHMInfo);
	KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[1], ret,"1111111111111111111111111111");
	if (ret != 0)
	{
		KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[1], ret,"func KeyMngsvr_DBOp_WriteSecKey() err:%d \n", ret);
		goto End;
	}
	KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[1], ret,"222222222222222222222222");
	 
End:
	
	KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[1], ret,"33333333333333333333333");
	if  (ret == 0)
	{
		KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[1], ret,"444444444444444444444");
		IC_DBApi_Commit(handle); //提交事物
	}
	else
	{
		KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[1], ret,"444444444444444444-2");
		IC_DBApi_Rollback(handle); //回退
	}
	KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[1], ret,"5555555555555555555555");
	
	//
	if (ret == IC_DB_CONNECT_ERR)
	{
		IC_DBApi_ConnFree(handle, 0); //需要断链修复
	}
	else
	{
		IC_DBApi_ConnFree(handle, 1); 	 //正常连接
	}
	
	
	
	KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[1], ret,"func MngServer_Agree() End ");
	return ret;
}


//服务器端 读自己的共享内存
//和客户端的数据 进行比较
//ok -- 0   err
int MngServer_Check(MngServer_Info *svrInfo, MsgKey_Req *msgkeyReq, unsigned char **outData, int *datalen)
{
	int					 i = 0, ret = 0;
 
	NodeSHMInfo			nodeInfo;
	MsgKey_Res			msgKeyRes;

	KeyMng_Log(__FILE__, __LINE__, KeyMngLevel[1], ret, "func MngServer_Agree() Begin ");

	memset(&msgKeyRes, 0, sizeof(MsgKey_Res));

	msgKeyRes.rv = 0;
	strcpy(msgKeyRes.clientId, msgkeyReq->clientId);
	strcpy(msgKeyRes.serverId, svrInfo->serverId);

	//解析获取数据
	//读取共享内存
	/*读网点密钥
	int KeyMng_ShmRead(int shmhdl, char *clientId, char *serverId, int maxnodenum, NodeSHMInfo *pNodeInfo)*/
	
	memset(&nodeInfo, 0, sizeof(nodeInfo));
	ret=KeyMng_ShmRead(svrInfo->shmhdl,msgkeyReq->clientId,svrInfo->serverId,svrInfo->maxnode, &nodeInfo);
	if (ret != 0)
	{
		KeyMng_Log(__FILE__, __LINE__, KeyMngLevel[4], ret, "func KeyMng_ShmRead() err:%d \n", ret);
		goto End;
	}

	msgKeyRes.seckeyid = nodeInfo.seckeyid;

	//把服务器的r2页发过去,虽然我不在客户端对比
	for (i = 0; i < 8; i++)
	{
		msgKeyRes.r2[i] = nodeInfo.seckey[i];
	}


	//判断结果比对
	if (memcmp(msgkeyReq->r1, nodeInfo.seckey, 8) == 0)
	{
		msgKeyRes.rv = 0;
	}
	else
	{
		msgKeyRes.rv = 100;
	}


	//编码应答报文
	ret = MsgEncode(&msgKeyRes, ID_MsgKey_Res, outData, datalen);
	if (ret != 0)
	{
		KeyMng_Log(__FILE__, __LINE__, KeyMngLevel[4], ret, "func MsgEncode() err ");
		goto End;
	}

	End:
	//ret返回
	
	return ret;	
}