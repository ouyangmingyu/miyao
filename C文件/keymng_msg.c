
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "keymng_msg.h"
#include "itcast_asn1_der.h"

int TeacherEncode_(Teacher *pTeacher, ITCAST_ANYBUF **outData)
{
	int					ret = 0;
	ITCAST_ANYBUF		*pTmp = NULL, *pHeadBuf = NULL;
	ITCAST_ANYBUF		*pTmpBuf = NULL;
	ITCAST_ANYBUF		*pOutData = NULL;
	unsigned char		*tmpout = NULL;
	int					tmpoutlen = 0;

	// 把C语言的buf 转化成  ITCAST_ANYBUF
	ret = DER_ITCAST_String_To_AnyBuf(&pTmpBuf, pTeacher->name, strlen(pTeacher->name));
	if (ret != 0)
	{
		printf("func DER_ITCAST_String_To_AnyBuf() err:%d \n", ret);
		return ret;
	}

	//编码 name
	ret = DER_ItAsn1_WritePrintableString(pTmpBuf, &pHeadBuf);
	if (ret != 0)
	{
		DER_ITCAST_FreeQueue(pTmpBuf);
		printf("func DER_ITCAST_String_To_AnyBuf() err:%d \n", ret);
		return ret;
	}
	//两个辅助指针变量 指向 同一个节点 
	pTmp = pHeadBuf;

	//编码 age
	ret = DER_ItAsn1_WriteInteger(pTeacher->age, &(pTmp->next));
	if (ret != 0)
	{
		DER_ITCAST_FreeQueue(pHeadBuf); //释放链表 以免内存泄漏
		printf("func DER_ItAsn1_WriteInteger() err:%d \n", ret);
		return ret;
	}
	pTmp = pTmp->next;

	//编码 p
	ret = EncodeChar(pTeacher->p, pTeacher->plen, &pTmp->next);
	if (ret != 0)
	{
		DER_ITCAST_FreeQueue(pHeadBuf); //释放链表 以免内存泄漏
		printf("func EncodeChar() err:%d \n", ret);
		return ret;
	}
	pTmp = pTmp->next;

	//编码 plen
	ret = DER_ItAsn1_WriteInteger(pTeacher->plen, &(pTmp->next));
	if (ret != 0)
	{
		DER_ITCAST_FreeQueue(pHeadBuf); //释放链表 以免内存泄漏
		printf("func DER_ItAsn1_WriteInteger() err:%d \n", ret);
		return ret;
	}

	ret = DER_ItAsn1_WriteSequence(pHeadBuf, &pOutData);
	if (ret != 0)
	{
		DER_ITCAST_FreeQueue(pHeadBuf); //释放链表 以免内存泄漏
		printf("func DER_ItAsn1_WriteSequence() err:%d \n", ret);
		return ret;
	}

	*outData = pOutData; //老师结构体 大节点 传出来
	DER_ITCAST_FreeQueue(pHeadBuf); //释放链表 以免内存泄漏

	/* 
	*out = (unsigned char *)malloc(pOutData->dataLen);
	if (*out == NULL)
	{
		ret = 2;
		DER_ITCAST_FreeQueue(pOutData);
		printf("func TeacherEncode() malloc err:%d \n", ret);
		return ret;
	}
	memcpy(*out, pOutData->pData, pOutData->dataLen); //把运算结果copy到新分配的内存块中
	*outlen = pOutData->dataLen;
	*/
	//DER_ITCAST_FreeQueue(pOutData);

	return ret;
}

int TeacherDecode_(unsigned char *indata, int inLen, Teacher **pStruct)
{
	int					ret = 0;
	ITCAST_ANYBUF		*pTmp = NULL, *pHead = NULL;
	ITCAST_ANYBUF		*pOutData = NULL;
	ITCAST_ANYBUF		*inAnyBuf = NULL;

	Teacher				*pTmpStru = NULL;

/*
	//把c语言buf 转化成 ITCAST_ANYBUF
	inAnyBuf = (ITCAST_ANYBUF *)malloc(sizeof(ITCAST_ANYBUF));
	if (inAnyBuf == NULL)
	{
		ret = 1;
		printf("func TeacherDecode malloc error: %d", ret);
		return ret;
	}
	memset(inAnyBuf, 0, sizeof(ITCAST_ANYBUF));

	inAnyBuf->pData = (unsigned char *)malloc(inLen);
	if (inAnyBuf->pData == NULL)
	{
		DER_ITCAST_FreeQueue(inAnyBuf);
		ret = 2;
		printf("func TeacherDecode malloc error: %d", ret);
		return ret;
	}
	memcpy(inAnyBuf->pData, indata, inLen);
	inAnyBuf->dataLen = inLen;
*/

	//将 ber 格式的字节流，转换成AnyBuf
	ret = DER_ITCAST_String_To_AnyBuf(&inAnyBuf, indata, inLen);
	if (ret != 0)
	{
		printf("func DER_ITCAST_String_To_AnyBuf error : %d", ret);
		return ret;
	}

	//解码 Teacher 结构体 得到4个节点
	ret = DER_ItAsn1_ReadSequence(inAnyBuf, &pHead);
	if (ret != 0)
	{
		DER_ITCAST_FreeQueue(inAnyBuf);
		printf("func DER_ItAsn1_ReadSequence() err:%d \n", ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(inAnyBuf);

	//把老师结构体给分配出来
	pTmpStru = (Teacher *)malloc(sizeof(Teacher));
	if (pTmpStru == NULL)
	{
		ret = 3;
		DER_ITCAST_FreeQueue(pHead); //释放链表
		printf("func malloc() err:%d \n", ret);
		return ret;
	}
	memset(pTmpStru, 0, sizeof(Teacher));

	pTmp = pHead;
	//解码 name
	ret = DER_ItAsn1_ReadPrintableString(pTmp, &pOutData);
	if (ret != 0)
	{
		DER_ITCAST_FreeQueue(pHead); //释放链表
		printf("func DER_ItAsn1_ReadPrintableString() err:%d \n", ret);
		return ret;
	}

	//给name变量赋值
	memcpy(pTmpStru->name, pOutData->pData, pOutData->dataLen);
	pTmp = pTmp->next; //跳到下一个节点
	DER_ITCAST_FreeQueue(pOutData);

	//解码age
	ret = DER_ItAsn1_ReadInteger(pTmp, &(pTmpStru->age));
	if (ret != 0)
	{
		DER_ITCAST_FreeQueue(pHead); //释放链表
		printf("func DER_ItAsn1_ReadInteger() err:%d \n", ret);
		return ret;
	}
	pTmp = pTmp->next; //跳到下一个节点

	//解码 p
	ret = DER_ItAsn1_ReadPrintableString(pTmp, &pOutData);
	if (ret != 0)
	{
		DER_ITCAST_FreeQueue(pHead); //释放链表
		printf("func DER_ItAsn1_ReadPrintableString() err:%d \n", ret);
		return ret;
	}
	pTmpStru->p = (char *)malloc(pOutData->dataLen + 1);
	if (pTmpStru->p == NULL)
	{
		ret = 4;
		printf("func malloc err:%d \n", ret);
		return ret;
	}
	memcpy(pTmpStru->p, pOutData->pData, pOutData->dataLen);
	pTmpStru->p[pOutData->dataLen] = '\0';


	pTmp = pTmp->next; //跳到下一个节点
	DER_ITCAST_FreeQueue(pOutData);

	//解码 plen
	ret = DER_ItAsn1_ReadInteger(pTmp, &(pTmpStru->plen));
	if (ret != 0)
	{
		DER_ITCAST_FreeQueue(pHead); //释放链表
		printf("func DER_ItAsn1_ReadInteger() err:%d \n", ret);
		return ret;
	}

	*pStruct = pTmpStru; //间接赋值

	return ret;
}

int TeacherDecode_Free(Teacher **pStruct)
{
	Teacher *tmp = NULL;
	if (pStruct == NULL)
	{
		return 0;
	}

	tmp = *pStruct;

	if (tmp != NULL)
	{
		free(tmp->p);
		free(tmp);
	}
	*pStruct = NULL; // 1 不但把指针所指向的内存给释放掉 2 同时把实参赋值 NULL 避免野指针
	return 0;
}

// 对 type 和 Teacher 结构体进行编码 封装。
int MsgEncode(
	void			*pStruct, /*in*/
	int				type,
	unsigned char	**outData, /*out*/
	int				*outLen)
{
	ITCAST_ANYBUF	*pHeadbuf = NULL, *pTemp = NULL;
	ITCAST_ANYBUF	*pOutData = NULL;
	int				ret = 0;

	if (pStruct == NULL && type < 0 || outData == NULL || outLen == NULL)
	{
		ret = KeyMng_ParamErr;
		printf("func MsgEncode() err:%d \n", ret);
		return ret;
	}

	ret = DER_ItAsn1_WriteInteger(type, &pHeadbuf);
	if (ret != 0)
	{
		printf("func DER_ItAsn1_WriteInteger()　err:%d \n", ret);
		return ret;
	}
	
	switch (type)
	{
	case ID_MsgKey_Teacher:
		//编码老师
		//Teacher_Encode(&t1, &myOut, &mOutlen);
		ret = TeacherEncode_((Teacher *)pStruct, &pTemp);
		break;

	case ID_MsgKey_Req:
		//密钥请求报文  编码
		//ret = MsgKey_ReqEncode(密钥请求结构体， Anybuf);
		break;

	case ID_MsgKey_Res:
		//密钥应答报文 编码
		//ret = MsgKey_ResEncode(密钥请求结构体， Anybuf);
		break;

	default:
		ret = KeyMng_TypeErr;
		printf("类型输入失败()　itype:%d err:%d \n", type, ret);
		break;
	}

	if (ret != 0)
	{
		DER_ITCAST_FreeQueue(pHeadbuf);
		printf("编码失败err:%d \n", ret);
		return ret;
	}

	pHeadbuf->next = pTemp; 

	//对type 和 结构体 再做一次 TLV
	ret = DER_ItAsn1_WriteSequence(pHeadbuf, &pOutData);
	if (ret != 0)
	{
		DER_ITCAST_FreeQueue(pHeadbuf);
		printf("func DER_ItAsn1_WriteSequence()　err:%d \n", ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(pHeadbuf);

	*outData = (unsigned char *)malloc(pOutData->dataLen);
	if (*outData == NULL)
	{
		DER_ITCAST_FreeQueue(pOutData);
		ret = KeyMng_MallocErr;
		printf("malloc()　err:%d \n", ret);
		return ret;
	}
	memcpy(*outData, pOutData->pData, pOutData->dataLen);
	*outLen = pOutData->dataLen;

	DER_ITCAST_FreeQueue(pOutData);

	return ret;
}


int MsgDecode(
	unsigned char *inData,/*in*/
	int           inLen,
	void          **pStruct /*out*/,
	int           *type /*out*/)
{
	ITCAST_ANYBUF		*pHeadBuf = NULL, *inAnyBuf = NULL;
	int					ret = 0;
	unsigned long		itype = 0;

	//将 ber 格式的字节流，转换成 AnyBuf
	ret = DER_ITCAST_String_To_AnyBuf(&inAnyBuf, inData, inLen);
	if (ret != 0)
	{
		printf("func DER_ITCAST_String_To_AnyBuf error : %d", ret);
		return ret;
	}

	ret = DER_ItAsn1_ReadSequence(inAnyBuf, &pHeadBuf);
	if (ret != 0)
	{
		printf("func DER_ItAsn1_ReadSequence() err:%d \n", ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(inAnyBuf);

	//解析 type
	ret = DER_ItAsn1_ReadInteger(pHeadBuf, &itype);
	if (ret != 0)
	{
		DER_ITCAST_FreeQueue(pHeadBuf); 
		printf("func DER_ItAsn1_ReadInteger() err:%d \n", ret);
		return ret;
	}

	switch (itype)
	{
	case ID_MsgKey_Teacher:
		//解码 老师结构体
		ret = TeacherDecode_(pHeadBuf->next->pData, pHeadBuf->next->dataLen, (Teacher **)pStruct);
		break;
	case ID_MsgKey_Req:
		//解码 请求报文 
		//ret = MsgKey_ReqDecode(pHeadBuf->next->pData, pHeadBuf->next->dataLen, (MsgKey_Req **)pStruct);
		break;
	case ID_MsgKey_Res:
		//解码 应答报文

		break;
	default:
		ret = KeyMng_TypeErr;
		printf("itype:%d失败 :%d \n", itype, ret);
		break;
	}

	if (ret != 0)
	{
		DER_ITCAST_FreeQueue(pHeadBuf);
		return ret;
	}

	*type = itype;
	DER_ITCAST_FreeQueue(pHeadBuf);

	return ret;
}


int MsgMemFree(void **point, int type)
{
	if (point == NULL)
	{
		return 0;
	}

	if (type == 0) //释放 Encode编码以后的内存块
	{ 
		if (*point) free(*point);
		*point = NULL;
		return 0;
	}

	switch (type)
	{
	case ID_MsgKey_Teacher:
		TeacherDecode_Free((Teacher **)point);
		break;
	case ID_MsgKey_Req:
		//MsgKey_Req_Free((Teacher **)point);
		break;

	default:
		break;
	}

	return 0;
}