
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

	// ��C���Ե�buf ת����  ITCAST_ANYBUF
	ret = DER_ITCAST_String_To_AnyBuf(&pTmpBuf, pTeacher->name, strlen(pTeacher->name));
	if (ret != 0)
	{
		printf("func DER_ITCAST_String_To_AnyBuf() err:%d \n", ret);
		return ret;
	}

	//���� name
	ret = DER_ItAsn1_WritePrintableString(pTmpBuf, &pHeadBuf);
	if (ret != 0)
	{
		DER_ITCAST_FreeQueue(pTmpBuf);
		printf("func DER_ITCAST_String_To_AnyBuf() err:%d \n", ret);
		return ret;
	}
	//��������ָ����� ָ�� ͬһ���ڵ� 
	pTmp = pHeadBuf;

	//���� age
	ret = DER_ItAsn1_WriteInteger(pTeacher->age, &(pTmp->next));
	if (ret != 0)
	{
		DER_ITCAST_FreeQueue(pHeadBuf); //�ͷ����� �����ڴ�й©
		printf("func DER_ItAsn1_WriteInteger() err:%d \n", ret);
		return ret;
	}
	pTmp = pTmp->next;

	//���� p
	ret = EncodeChar(pTeacher->p, pTeacher->plen, &pTmp->next);
	if (ret != 0)
	{
		DER_ITCAST_FreeQueue(pHeadBuf); //�ͷ����� �����ڴ�й©
		printf("func EncodeChar() err:%d \n", ret);
		return ret;
	}
	pTmp = pTmp->next;

	//���� plen
	ret = DER_ItAsn1_WriteInteger(pTeacher->plen, &(pTmp->next));
	if (ret != 0)
	{
		DER_ITCAST_FreeQueue(pHeadBuf); //�ͷ����� �����ڴ�й©
		printf("func DER_ItAsn1_WriteInteger() err:%d \n", ret);
		return ret;
	}

	ret = DER_ItAsn1_WriteSequence(pHeadBuf, &pOutData);
	if (ret != 0)
	{
		DER_ITCAST_FreeQueue(pHeadBuf); //�ͷ����� �����ڴ�й©
		printf("func DER_ItAsn1_WriteSequence() err:%d \n", ret);
		return ret;
	}

	*outData = pOutData; //��ʦ�ṹ�� ��ڵ� ������
	DER_ITCAST_FreeQueue(pHeadBuf); //�ͷ����� �����ڴ�й©

	/* 
	*out = (unsigned char *)malloc(pOutData->dataLen);
	if (*out == NULL)
	{
		ret = 2;
		DER_ITCAST_FreeQueue(pOutData);
		printf("func TeacherEncode() malloc err:%d \n", ret);
		return ret;
	}
	memcpy(*out, pOutData->pData, pOutData->dataLen); //��������copy���·�����ڴ����
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
	//��c����buf ת���� ITCAST_ANYBUF
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

	//�� ber ��ʽ���ֽ�����ת����AnyBuf
	ret = DER_ITCAST_String_To_AnyBuf(&inAnyBuf, indata, inLen);
	if (ret != 0)
	{
		printf("func DER_ITCAST_String_To_AnyBuf error : %d", ret);
		return ret;
	}

	//���� Teacher �ṹ�� �õ�4���ڵ�
	ret = DER_ItAsn1_ReadSequence(inAnyBuf, &pHead);
	if (ret != 0)
	{
		DER_ITCAST_FreeQueue(inAnyBuf);
		printf("func DER_ItAsn1_ReadSequence() err:%d \n", ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(inAnyBuf);

	//����ʦ�ṹ����������
	pTmpStru = (Teacher *)malloc(sizeof(Teacher));
	if (pTmpStru == NULL)
	{
		ret = 3;
		DER_ITCAST_FreeQueue(pHead); //�ͷ�����
		printf("func malloc() err:%d \n", ret);
		return ret;
	}
	memset(pTmpStru, 0, sizeof(Teacher));

	pTmp = pHead;
	//���� name
	ret = DER_ItAsn1_ReadPrintableString(pTmp, &pOutData);
	if (ret != 0)
	{
		DER_ITCAST_FreeQueue(pHead); //�ͷ�����
		printf("func DER_ItAsn1_ReadPrintableString() err:%d \n", ret);
		return ret;
	}

	//��name������ֵ
	memcpy(pTmpStru->name, pOutData->pData, pOutData->dataLen);
	pTmp = pTmp->next; //������һ���ڵ�
	DER_ITCAST_FreeQueue(pOutData);

	//����age
	ret = DER_ItAsn1_ReadInteger(pTmp, &(pTmpStru->age));
	if (ret != 0)
	{
		DER_ITCAST_FreeQueue(pHead); //�ͷ�����
		printf("func DER_ItAsn1_ReadInteger() err:%d \n", ret);
		return ret;
	}
	pTmp = pTmp->next; //������һ���ڵ�

	//���� p
	ret = DER_ItAsn1_ReadPrintableString(pTmp, &pOutData);
	if (ret != 0)
	{
		DER_ITCAST_FreeQueue(pHead); //�ͷ�����
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


	pTmp = pTmp->next; //������һ���ڵ�
	DER_ITCAST_FreeQueue(pOutData);

	//���� plen
	ret = DER_ItAsn1_ReadInteger(pTmp, &(pTmpStru->plen));
	if (ret != 0)
	{
		DER_ITCAST_FreeQueue(pHead); //�ͷ�����
		printf("func DER_ItAsn1_ReadInteger() err:%d \n", ret);
		return ret;
	}

	*pStruct = pTmpStru; //��Ӹ�ֵ

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
	*pStruct = NULL; // 1 ������ָ����ָ����ڴ���ͷŵ� 2 ͬʱ��ʵ�θ�ֵ NULL ����Ұָ��
	return 0;
}

// �� type �� Teacher �ṹ����б��� ��װ��
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
		printf("func DER_ItAsn1_WriteInteger()��err:%d \n", ret);
		return ret;
	}
	
	switch (type)
	{
	case ID_MsgKey_Teacher:
		//������ʦ
		//Teacher_Encode(&t1, &myOut, &mOutlen);
		ret = TeacherEncode_((Teacher *)pStruct, &pTemp);
		break;

	case ID_MsgKey_Req:
		//��Կ������  ����
		//ret = MsgKey_ReqEncode(��Կ����ṹ�壬 Anybuf);
		break;

	case ID_MsgKey_Res:
		//��ԿӦ���� ����
		//ret = MsgKey_ResEncode(��Կ����ṹ�壬 Anybuf);
		break;

	default:
		ret = KeyMng_TypeErr;
		printf("��������ʧ��()��itype:%d err:%d \n", type, ret);
		break;
	}

	if (ret != 0)
	{
		DER_ITCAST_FreeQueue(pHeadbuf);
		printf("����ʧ��err:%d \n", ret);
		return ret;
	}

	pHeadbuf->next = pTemp; 

	//��type �� �ṹ�� ����һ�� TLV
	ret = DER_ItAsn1_WriteSequence(pHeadbuf, &pOutData);
	if (ret != 0)
	{
		DER_ITCAST_FreeQueue(pHeadbuf);
		printf("func DER_ItAsn1_WriteSequence()��err:%d \n", ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(pHeadbuf);

	*outData = (unsigned char *)malloc(pOutData->dataLen);
	if (*outData == NULL)
	{
		DER_ITCAST_FreeQueue(pOutData);
		ret = KeyMng_MallocErr;
		printf("malloc()��err:%d \n", ret);
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

	//�� ber ��ʽ���ֽ�����ת���� AnyBuf
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

	//���� type
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
		//���� ��ʦ�ṹ��
		ret = TeacherDecode_(pHeadBuf->next->pData, pHeadBuf->next->dataLen, (Teacher **)pStruct);
		break;
	case ID_MsgKey_Req:
		//���� ������ 
		//ret = MsgKey_ReqDecode(pHeadBuf->next->pData, pHeadBuf->next->dataLen, (MsgKey_Req **)pStruct);
		break;
	case ID_MsgKey_Res:
		//���� Ӧ����

		break;
	default:
		ret = KeyMng_TypeErr;
		printf("itype:%dʧ�� :%d \n", itype, ret);
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

	if (type == 0) //�ͷ� Encode�����Ժ���ڴ��
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