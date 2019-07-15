#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <pthread.h>


#include "keymng_dbop.h"
#include "keymngserverop.h"
#include "keymnglog.h"
#include "icdbapi.h"

//读keysn 
int KeyMngsvr_DBOp_GenKeyID(void *dbhdl, int *keyid)
{
	int 			ret = 0;
	int 			ikeysn;
	
	ICDBRow     	row;
    ICDBField   	field[7];
    char			mysql1[1024];
    char			mysql2[1024];

    memset(field, 0, sizeof(field));
    memset(mysql1, 0, sizeof(mysql1));   
   	memset(mysql2, 0, sizeof(mysql2));   

	if (dbhdl== NULL || keyid==NULL)
	{
		ret = -1;
		KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[4], ret,"func KeyMngsvr_DBOp_GenKeyID() err");
       	goto END;
	}
	
	sprintf(mysql1, "select ikeysn from SECMNG.KEYSN for update ");//使用sql锁 for update  
	
	// 读取序列号累加器
    field[0].cont = (char *)&ikeysn;
                
    row.field = field;
    row.fieldCount = 1;	
    // 累加器加1 
	sprintf(mysql2,"update SECMNG.KEYSN set ikeysn = ikeysn + 1");  
	
    ret =  IC_DBApi_ExecSelSql(dbhdl, mysql1, &row);    
	if (ret != 0)
	{
    	KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[4], ret,"func IC_DBApi_ExecSelSql() err,\n sql:%s", mysql1);
    	goto END;	
	}	
 
    ret =  IC_DBApi_ExecNSelSql(dbhdl, mysql2); //执行单个非select语言
    if (ret != 0)
    {
    	KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[4], ret,"func IC_DBApi_ExecNSelSql() err, sql:%s", mysql2);
    	goto END;
    }
    *keyid = ikeysn; 
  
END:
	
	return ret;
}

//测试PEM和DER互转
int IC_DBApi_Pem2Der_Test2()
{
	int	 			rv = -1;
	char 			str[1024];
	char 			strPem[1024];
	unsigned char 	strDer[1024];
	int	 			strPemLen = 1024;
	int	 			strDerLen = 1024;
	
	memset(str, 0, 1024);
	memset(strPem, 0, 1024);
	memset(strDer, 0, 1024);
	
	printf("请输入要从DER转化为PEM的字符串：");
	//fgets(str, 256, stdin);
//	scanf("%s", str);
//	while(getchar() != '\n');

	strcpy(str, "abcdefg");
    
    //DER2PEM
    rv = IC_DBApi_Der2Pem((unsigned char*)str, strlen(str), strPem, &strPemLen);
    if(rv)
    {
    	printf("DER2PEM ERROR!\n");
    	return rv;
    }
    else
    {
    	printf("DER2PEM OK!PEM is %s\n", strPem);
    }
    
    //PEM2DER
    rv = IC_DBApi_Pem2Der(strPem, strPemLen, strDer, &strDerLen);
    if(rv)
    {
    	printf("PEM2DER ERROR!\n");
    	return rv;
    }
    else
    {
    	printf("PEM2DER OK!DER is %s\n", strDer);
    }
	
	return 0;
}

//写数据库
int KeyMngsvr_DBOp_WriteSecKey(void *dbhdl, NodeSHMInfo *pNodeInfo) 
{
	int         	ret = 0;
    char        	mysql[2048] = {0};
    
    char			optime[24]  = {0};
    char			tmpseckey2[1024];
    int				tmplen = 1024;
    char			buf2[1024];
    
    memset(tmpseckey2, 0, sizeof(tmpseckey2));
    memset(mysql, 0, sizeof(mysql));
    
    // 获取当前操作时间
    ret = IC_DBApi_GetDBTime(dbhdl, optime);
    if (ret != 0)
    {
    	KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[4], ret,"func IC_DBApi_GetDBTime() err");
       	goto END;
    }
    
   
    //varchar2 4096 //ora9
	ret = IC_DBApi_Der2Pem( pNodeInfo->seckey, sizeof(pNodeInfo->seckey) , tmpseckey2, &tmplen );
    if (ret != 0)
    {
    	KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[4], ret,"func IC_DBApi_Der2Pem() err\n");
       	goto END;
    }
    //printf("tmpseckey:%s \n", tmpseckey2);
    //printf("tmplen:%d \n", tmplen);
    // aabbccde //...........434434343
    
    KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[2], ret, "\ntmpseckey===>%s \n tmplen===>%d\n", tmpseckey2, tmplen);
   	
    //组织sql语句
	sprintf(mysql, "Insert Into SECMNG.SECKYEINFO(clientid, serverid, keyid, createtime, state, seckey) \
					values ('%s', '%s', %d, '%s', %d, '%s') ", pNodeInfo->clientId,  pNodeInfo->serverId, \
					pNodeInfo->seckeyid, optime, 0, tmpseckey2 );
   
    //执行非select sql语句
    ret  = IC_DBApi_ExecNSelSql(dbhdl, mysql);
    if (ret != 0)
    {
    	KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[4], ret, "func IC_DBApi_ExecNSelSql() err \n sql===>%s\n", mysql);
        goto END;
    }
  
 END:
	
	return ret;

}

int KeyMngsvr_DBOp_WriteSecKey_ERR(void *dbhdl, NodeSHMInfo *pNodeInfo) 
{
	int         	ret = 0;
    char        	mysql[2048] = {0};
    
    char			optime[24]  = {0};
    char			tmpseckey2[1024];
    int				tmplen = 1024;
    	char buf2[1024];
    
    void *handle = NULL;
    memset(tmpseckey2, 0, sizeof(tmpseckey2));
    memset(mysql, 0, sizeof(mysql));
    
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
	
    
      // 获取当前操作时间
    ret = IC_DBApi_GetDBTime(handle, optime);
    if (ret != 0)
    {
    	KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[4], ret,"func IC_DBApi_GetDBTime() err");
       	goto End;
    }
    
   /*
    
    {
   		 	//不可见字符 进行编码
	  	char buf1[100];
	  
	  	strcpy(buf1, "abcdefg");
	  	
	   	//ret = IC_DBApi_Der2Pem( (unsigned char *)"abcdefg", strlen("abcdefg") , tmpseckey2, &tmplen );
	   	ret = IC_DBApi_Der2Pem( buf1, strlen(buf1) , tmpseckey2, &tmplen );
	    if (ret != 0)
	    {
	    	KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[4], ret,"func IC_DBApi_Der2Pem() err\n");
	       	goto END;
	    }
	    printf("tmpseckey:%s \n", tmpseckey2);
	     printf("tmplen:%d \n", tmplen);
	     
	    ret = IC_DBApi_Der2Pem( buf1, strlen(buf1) , buf2, &tmplen );
	    if (ret != 0)
	    {
	    	KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[4], ret,"func IC_DBApi_Der2Pem() err\n");
	       	goto END;
	    }
    
   	}
   	
    
    {
    	int	 			rv = -1;
		char 			strPem[1024];
		unsigned char 	strDer[1024];
		int	 			strPemLen = 1024;
		int	 			strDerLen = 1024;
    	char 			str[1024];
    	strcpy(str, "abcdefg");
    	rv = IC_DBApi_Der2Pem((unsigned char*)str, strlen(str), strPem, &strPemLen);
	   	if(rv)
	    {
	    	printf("DER2PEM ERROR!\n");
	    	return rv;
	    }
	    else
	    {
	    	printf("strPem: %s\n", strPem);
	    }
	    	//blob clob 
    }*/
    
    //varchar2 4096 //ora9
	ret = IC_DBApi_Der2Pem( pNodeInfo->seckey, sizeof(pNodeInfo->seckey) , tmpseckey2, &tmplen );
    if (ret != 0)
    {
    	KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[4], ret,"func IC_DBApi_Der2Pem() err\n");
       	goto End;
    }
    printf("tmpseckey:%s \n", tmpseckey2);
    printf("tmplen:%d \n", tmplen);
   // aabbccde //...........434434343
   KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[1], ret,"aaaaaaaaaaaaaaaaaaaaa1111");
    
    KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[2], ret, "\ntmpseckey===>%s \n tmplen===>%d\n", tmpseckey2, tmplen);
   	
     //组织sql语句
	sprintf(mysql, "Insert Into SECMNG.SECKYEINFO(clientid, serverid, keyid, createtime, state, seckey) \
					values ('%s', '%s', %d, '%s', %d, '%s') ", pNodeInfo->clientId,  pNodeInfo->serverId, \
					pNodeInfo->seckeyid, optime, 0, tmpseckey2 );
   
    //执行非select sql语句
    ret  = IC_DBApi_ExecNSelSql(handle, mysql);
    if (ret != 0)
    {
    	KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[4], ret, "func IC_DBApi_ExecNSelSql() err \n sql===>%s\n", mysql);
        goto End;
    }
    
     KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[1], ret,"aaaaaaaaaaaaaaaaaaaaa22222222222222");
  
   End:
   	
 	if  (ret == 0)
	{
		IC_DBApi_Commit(handle); //提交事物
		 KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[1], ret,"aaaaaaaaaaaaaaaaaaaaa333333333333333333333");
	}
	else
	{
		 KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[1], ret,"aaaaaaaaaaaaaaaaaaaaaa444444444444444");
		IC_DBApi_Rollback(handle); //回退
	}
	
	 KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[1], ret,"aaaaaaaaaaaaaaaaaaaaa55555555555555");
	//
	if (ret == IC_DB_CONNECT_ERR)
	{
		 KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[1], ret,"aaaaaaaaaaaaaaaaaaaaaaaaaaa66666666666666");
		IC_DBApi_ConnFree(handle, 0); //需要断链修复
	}
	else
	{
		KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[1], ret,"aaaaaaaaaaaaaaaaaaaaaa777777777777777");
		IC_DBApi_ConnFree(handle, 1); 	 //正常连接
	}

 	
 		KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[1], ret,"aaaaaaaaaaaaaaaaaaa8888888888888888");
 	IC_DBApi_PoolFree();
 	KeyMng_Log(__FILE__, __LINE__,KeyMngLevel[1], ret,"aaaaaaaaaaaaaaaaaaaaaaaaaa999999999999");
	
	return ret;

}