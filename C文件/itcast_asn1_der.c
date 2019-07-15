#define  _CRT_SECURE_NO_WARNINGS 
#include <stdio.h>
#include <time.h>

#include <string.h>
#include <locale.h>
#include <stdlib.h>

#include "itcast_asn1_der.h"
#include "itcastderlog.h"


#define OIDDEF(tag, oidValue) {tag, {oidValue,sizeof(oidValue),0,1,0,0,0} }


ITCAST_INT
DER_ItAsn1_Low_GetTagInfo(
    ITCAST_UINT8 **ppDerData,
    ITCAST_UINT32 **ppTagValue,
    ITCAST_UINT32 **ppTagSize)
{
	ITCAST_UINT8 *pMidData = NULL;
	ITCAST_UINT32 *pMidValue = NULL;
	ITCAST_UINT32 *pMidSize = NULL;

	//��ʼ��
	pMidValue = malloc(sizeof(ITCAST_UINT32));
	if(pMidValue == NULL)
	{
		ITDER_LOG(__FILE__, __LINE__,LogLevel[4], ITDER_MemoryErr,"func DER_ItAsn1_Low_GetTagInfo() err");
		return ITDER_MemoryErr;    
	}
	pMidSize = malloc(sizeof(ITCAST_UINT32));
	if(pMidSize == NULL)
	{
		if (pMidValue) {free(pMidValue); pMidValue=NULL;}
		ITDER_LOG(__FILE__, __LINE__,LogLevel[4], ITDER_MemoryErr,"func DER_ItAsn1_Low_GetTagInfo() err" );
		return ITDER_MemoryErr;	
	}
		
	*pMidSize = 0;
	*pMidValue = 0;
	pMidData = *ppDerData;
    //��Tag
	if ((*pMidData & ITCAST_DER_SHORT_ID_MASK) != ITCAST_DER_SHORT_ID_MASK)
	{
		if (*pMidData & ITCAST_DER_CONTEXT_SPECIFIC)
            *pMidValue = *(pMidData++);
		else
			*pMidValue = *(pMidData++) & ITCAST_DER_SHORT_ID_MASK;
		(*pMidSize)++;
	}
	else
	{
		do
		{
			*pMidValue = *pMidValue | (*(++pMidData) & ITCAST_DER_FIRST_NOT_ID_MASK);
            *pMidValue<<= 8;
			(*pMidSize)++;
		}
		while (!(*pMidData & ITCAST_DER_FIRST_YES_ID_MASK));
		*pMidValue |= *(pMidData++);
	    (*pMidSize)++;
	}
	//�������
    *ppTagValue = pMidValue;
    *ppTagSize = pMidSize;
    *ppDerData = pMidData;
    //�м��������
	pMidValue = NULL;
	pMidSize = NULL;
	pMidData = NULL;
	
	return 0;
}

ITCAST_UINT32
DER_ItAsn1_Low_Count_LengthOfSize(
	ITCAST_UINT32 iLength)
{
	if (iLength <= 0x7F)
		return (1);
	else
		if (iLength <= 0xFF)
			return (2);
		else
			if (iLength < 0xFFFF)
				return (3);
			else 
				if (iLength <= 0xFFFFFF)
					return (4);
				else
					if (iLength <= 0xFFFFFFFF)
						return (5);
					else
					{
						ITDER_LOG(__FILE__, __LINE__,LogLevel[4], ITDER_LengthErr,"func DER_ItAsn1_Low_Count_LengthOfSize() err");
						return ITDER_LengthErr;	
					}
}

ITCAST_INT
DER_ItAsn1_GetLengthInfo(
    ITCAST_ANYBUF *pDerData,
    int *pLengthValue,
    int *pLengthSize)
{
	ITCAST_UINT8 *pData;
	int iSizeOf,iSize = 0,i;
	pData = pDerData->pData;
	if (*pData & ITCAST_DER_SHORT_ID_MASK != ITCAST_DER_SHORT_ID_MASK)
		pData++;
	else
	{
		++pData;
		do
		{
			++pData;
		}
		while(!(*pData & 80));
	}
	++pData;
	iSizeOf = *pData & ITCAST_DER_FIRST_NOT_ID_MASK;
	if (!(*pData & 80))
	{
		*pLengthSize = 1;
        *pLengthValue = iSizeOf;
	}
	else
	{
		for(i=1;i <= iSizeOf;i++)
		{
			iSize |= *(++pData);
			iSize <<= 8;
		}
        *pLengthSize = iSizeOf;
        *pLengthValue = iSize;
	}
    return 0;
}

ITCAST_INT
DER_ItAsn1_Low_GetLengthInfo(
     ITCAST_UINT8 **ppDerData,
     ITCAST_UINT32 **ppLengthValue,
     ITCAST_UINT32 **ppLengthSize)
{
    ITCAST_UINT8 *pMidData,cSizeOf;
	ITCAST_UINT32 lMidLength = 0,*pMidLength,*pSizeOf,i; 

    //��ʼ��
	DER_ITASN1_LOW_CREATEUINT32(pSizeOf);
	DER_ITASN1_LOW_CREATEUINT32(pMidLength);
	pMidData = *ppDerData;
	//������
	if (!(*pMidData & ITCAST_DER_FIRST_YES_ID_MASK))//short
	{
		cSizeOf = 1;
		lMidLength =(ITCAST_UINT32)(*(pMidData++) & ITCAST_DER_FIRST_NOT_ID_MASK);
	}
	else                                //long
	{
		cSizeOf = *(pMidData++) & ITCAST_DER_FIRST_NOT_ID_MASK;
		if (cSizeOf >4/* ||cSizeOf <0*/)
		{
			if (pSizeOf) {free(pSizeOf); pSizeOf=NULL;}
			if (pMidLength)
			{
				free(pMidLength);
			}
			ITDER_LOG(__FILE__, __LINE__,LogLevel[4], ITDER_LengthErr,"func DER_ItAsn1_Low_GetLengthInfo() err");
			return ITDER_LengthErr;
		}
		else
		{
		    for (i = 1;i<cSizeOf;i++)
			{
			     lMidLength |=(ITCAST_UINT32)*(pMidData++);
			     lMidLength <<= 8;
			}
            lMidLength |=(ITCAST_UINT32)*(pMidData++);
			cSizeOf ++;
		}
	}
	//�����Ϣ
	*pMidLength = lMidLength;
	*pSizeOf = cSizeOf;
	*ppLengthValue = pMidLength;
	*ppLengthSize = pSizeOf;
	*ppDerData = pMidData;
    //�м��������
	pMidData = NULL;
	pMidLength = NULL;
	pSizeOf = NULL;

    return 0;
}


//��һ��ITCAST_UINT32���͵�����ת�����ַ���ʾ��ʽ
ITCAST_INT
DER_ItAsn1_Low_IntToChar(
	ITCAST_UINT32 integer,
	ITCAST_UINT8 **ppData,
	ITCAST_UINT32 **ppLength)
{
	ITCAST_UINT8 *pMidData = NULL,*pMidSite = NULL;
	ITCAST_UINT32 *pMidLength = NULL;
	ITCAST_UINT32  iValue;
	
	//��ʼ��
	iValue = integer;
    DER_ITASN1_LOW_CREATEUINT32(pMidLength);
	if (iValue < 0x0 || iValue > 0xFFFFFFFF)//�������ֵ
		return ITDER_DataRangeErr;
	else
	{
		if (iValue > 0xFFFFFF)
		{
			if (iValue >= 0x80000000)
			{
				DER_ITASN1_LOW_CREATEUINT8(pMidData,5);
				pMidSite = pMidData;
				*(pMidData++) = 0;
				*pMidLength = 1;				
			}
			else 
			{
				DER_ITASN1_LOW_CREATEUINT8(pMidData,4);
				pMidSite = pMidData;
			}
       	   *(pMidData++) = (ITCAST_UINT8)(iValue >>24);
		   *(pMidData++) = (ITCAST_UINT8)(iValue >>16);
		   *(pMidData++) = (ITCAST_UINT8)(iValue >>8);
		   *(pMidData++) = (ITCAST_UINT8)(iValue);
		   (*pMidLength) += 4;
		}
		else
		if (iValue > 0xFFFF)
		{
			if (iValue >= 0x800000)
			{
				DER_ITASN1_LOW_CREATEUINT8(pMidData,4);
				pMidSite = pMidData;
				*(pMidData++) = 0;
				*pMidLength = 1;
			}
			else 
			{
				DER_ITASN1_LOW_CREATEUINT8(pMidData,3);
				pMidSite = pMidData;
			}
			*(pMidData++) = (ITCAST_UINT8)(iValue >>16);
			*(pMidData++) = (ITCAST_UINT8)(iValue >>8);
           	*(pMidData++) = (ITCAST_UINT8)(iValue);
            (*pMidLength) += 3;
		}
		else
		if (iValue > 0xFF)
		{
			if (iValue >= 0x8000)
			{
				DER_ITASN1_LOW_CREATEUINT8(pMidData,3);
				pMidSite = pMidData;
			    *(pMidData++) = 0;
				*pMidLength = 1;
			}
			else
			{
				DER_ITASN1_LOW_CREATEUINT8(pMidData,2);
				pMidSite = pMidData;
			}
			*(pMidData++) = (ITCAST_UINT8)(iValue >>8);
			*(pMidData++) = (ITCAST_UINT8)(iValue);
			(*pMidLength) += 2;
		}
		else
		if (iValue >= 0x0)
		{
			if (iValue >= 0x80) 
			{
				DER_ITASN1_LOW_CREATEUINT8(pMidData,2);
				pMidSite = pMidData;
			    *(pMidData++) = 0;
			    *pMidLength = 1;
			}
			else
			{
				DER_ITASN1_LOW_CREATEUINT8(pMidData,1);
				pMidSite = pMidData;
			}
            *pMidData = (ITCAST_UINT8)(iValue);
			(*pMidLength) += 1;
		}
	}
	//��Ϣ���
	*ppData = pMidSite;
	*ppLength = pMidLength;
    //�м��������
	pMidData = NULL;
	pMidSite = NULL;
	pMidLength = NULL;

 	return 0;
}

//��һ�����ַ���ʾ������ת����ITCAST_UINT32������
ITCAST_INT
DER_ItAsn1_Low_CharToInt(
	ITCAST_UINT8 *aData,
	ITCAST_UINT32 lLength,
	ITCAST_UINT32 **ppInteger)
{
	ITCAST_UINT32 lIntMid = 0,i;
	ITCAST_UINT32 *pIntMid = NULL;
   
	DER_ITASN1_LOW_CREATEUINT32(pIntMid);
	//ת��
	if ((*aData == 0)&&(lLength > 1))
	{
		aData++;
		lLength--;
	}
	for (i = 1;i < lLength;i++)
	{
		
		lIntMid |= *aData++;
		lIntMid <<= 8;
	}
    lIntMid |= *aData++;
	//���
	*pIntMid = lIntMid;
	*ppInteger = pIntMid;
    //�м��������
	pIntMid = NULL;

    return 0;
}


//дTag��Lengthֵ
ITCAST_INT
DER_ItAsn1_Low_WriteTagAndLength(
    ITCAST_ANYBUF *pAnyIn,
	ITCAST_UINT8 cTag,
	ITCAST_ANYBUF **ppAnyOut,
	ITCAST_UINT8 **ppUint8Value)
{
	ITCAST_ANYBUF *pMidAny = NULL;
	ITCAST_UINT8 *pMidValue = NULL,cIdentifier;
	ITCAST_UINT32 iMidSize,iMidSizeOf,iMidLength,i;
	
	//���㳤��,Bitstring��Integer�������������ͷֿ�����
	if ((cTag != ITCAST_DER_ID_BITSTRING) && (cTag != ITCAST_DER_ID_INTEGER))
	    iMidSize = pAnyIn ->dataLen;
	else
		if (cTag == ITCAST_DER_ID_INTEGER)
			if (!(*(pAnyIn ->pData) & ITCAST_DER_FIRST_YES_ID_MASK))
				iMidSize = pAnyIn ->dataLen;
			else
				iMidSize = pAnyIn ->dataLen + 1;
		else 
				iMidSize = pAnyIn ->dataLen + 1;
	iMidLength = iMidSize;
	iMidSizeOf = DER_ItAsn1_Low_Count_LengthOfSize(iMidSize);
	if(iMidSizeOf < 0 || iMidSizeOf > 5)
	{
		ITDER_LOG(__FILE__, __LINE__,LogLevel[4], ITDER_LengthErr,"func DER_ItAsn1_Low_WriteTagAndLength() err");
		return ITDER_LengthErr;
	}
	iMidSize += 1 + iMidSizeOf;
    DER_CREATE_LOW_ITCAST_ANYBUF(pMidAny);
	DER_ITASN1_LOW_CREATEUINT8(pMidValue,iMidSize);
	if (iMidSize == 11)
	{
		iMidSize = 11;
	}
	pMidAny ->pData = pMidValue;
	//���Tagֵ
	if (cTag & ITCAST_DER_CONTEXT_SPECIFIC) 
        cIdentifier = cTag;
	else
	{
        cIdentifier = DER_ITASN1_LOW_IDENTIFIER(cTag);
	    //if (cIdentifier != pAnyIn ->dataType)
		//    return ITDER_MemoryErr;
	}
	
	*(pMidValue++)= cTag;
	pMidAny ->dataType =(ITCAST_UINT32)cIdentifier;
	pMidAny ->dataLen = iMidSize;
    if (iMidSizeOf == 1)
	{
		*(pMidValue++)= (ITCAST_UINT8)(iMidLength);
	}
	else
	{
		*(pMidValue++) = ITCAST_DER_FIRST_YES_ID_MASK | (ITCAST_UINT8)(iMidSizeOf-1);
        for (i = iMidSizeOf - 1;i > 0 ;i--)
		{
			 *(pMidValue++)= (ITCAST_UINT8)(iMidLength >>8*(i - 1));
		}
	}
	if (pMidAny ->dataType == ITCAST_DER_ID_BITSTRING)
	{
		pMidAny ->unusedBits = pAnyIn ->unusedBits;
        *(pMidValue++) = (ITCAST_UINT8)(pAnyIn ->unusedBits);
	}
	if (pMidAny ->dataType == ITCAST_DER_ID_INTEGER)
	{
		if (*(pAnyIn ->pData) & ITCAST_DER_FIRST_YES_ID_MASK)
		    *(pMidValue++) = 0x0;
	}
	//�����Ϣ
	*ppUint8Value = pMidValue;
    *ppAnyOut = pMidAny;
	
	pMidAny = NULL;
	pMidValue = NULL;
	
    return 0;
}

//��Tag��Lengthֵ
ITCAST_INT
DER_ItAsn1_Low_ReadTagAndLength(
	ITCAST_ANYBUF *pAnyIn,
	ITCAST_UINT8 **ppUint8Data,
	ITCAST_ANYBUF **ppAnyOut,
	ITCAST_UINT8 **ppUint8Value)
{
	ITCAST_ANYBUF *pMidAny = NULL;
	ITCAST_UINT32 *pMidTag = NULL;
	ITCAST_UINT8 *pMidValue = NULL;
	ITCAST_UINT32 *pMidSize = NULL,*pMidSizeOf = NULL,iMidLength = 0;

	DER_CREATE_LOW_ITCAST_ANYBUF(pMidAny);
	DER_ItAsn1_Low_GetTagInfo(ppUint8Data,&pMidTag,&pMidSize);
    //���Tagֵ�Ƿ���ȷ
	if (!(*pMidTag == ITCAST_DER_ID_STRING_PRINTABLE) || (*pMidTag == ITCAST_DER_ID_STRING_BMP))
	{
		/***************************************
		*****  if (*pMidTag != pAnyIn ->dataType)
        ****    return ITDER_MemoryErr;
		***************************************wyy*/
	}
	pMidAny->dataType = *pMidTag;
	iMidLength += *pMidSize;
	DER_ITCAST_Free(pMidSize);
    DER_ItAsn1_Low_GetLengthInfo(ppUint8Data,&pMidSize,&pMidSizeOf);
	//����ܳ����Ƿ���ȷ
	iMidLength +=*pMidSize + *pMidSizeOf;
	if (iMidLength != pAnyIn ->dataLen)
	{
		DER_ITCAST_Free(pMidSize);
		DER_ITCAST_Free(pMidSizeOf);
		DER_ITCAST_Free(pMidTag);	
		DER_ITCAST_Free(pMidAny);
		ITDER_LOG(__FILE__, __LINE__,LogLevel[4], ITDER_LengthNotEqualErr,"func DER_ItAsn1_Low_ReadTagAndLength() err");
		return ITDER_LengthNotEqualErr;

	}
	
	//Bitstring��Integer�������������Ͳ�ͬ
	if (pAnyIn ->dataType == ITCAST_DER_ID_BITSTRING)
	{
		pMidAny ->unusedBits = pAnyIn ->unusedBits;
		//���unusedBitsֵ�Ƿ���ȷ
		//if ((ITCAST_UINT8)(pMidAny ->unusedBits) != **ppUint8Data)
		//	return ITDER_MemoryErr;
		(*ppUint8Data)++;
		(*pMidSize)--;
	}
	if (*pMidTag == ITCAST_DER_ID_INTEGER)
	{
		pMidValue = *ppUint8Data;
		if ((*pMidValue == 0x0) && (*(++pMidValue) & ITCAST_DER_FIRST_YES_ID_MASK))
		{
			(*ppUint8Data)++;
		    (*pMidSize)--;
		}
		pMidValue = NULL;
	}
	DER_ITCAST_Free(pMidTag);
	//����pMidAny��pData�ռ�
	if (*pMidSize > 0)
	{
		DER_ITASN1_LOW_CREATEUINT8(pMidValue,*pMidSize);
		if (pMidValue == NULL)
		{
			DER_ITCAST_Free(pMidSize);
			DER_ITCAST_Free(pMidSizeOf);
			DER_ITCAST_Free(pMidAny);
			return ITDER_MemoryErr;
		}
	}
	else
	{
		pMidValue = NULL;
	}
	
	//���
	*ppUint8Value = pMidValue;
	pMidAny ->dataLen = *pMidSize;
	pMidAny ->pData = *ppUint8Value;
	*ppAnyOut = pMidAny;
    DER_ITCAST_Free(pMidSize);
	DER_ITCAST_Free(pMidSizeOf);

	//�м��������
	pMidAny = NULL;
	pMidValue = NULL;
	
 	return 0;
}

//DER������������
ITCAST_INT
DER_ItAsn1_WriteInteger(
    ITCAST_UINT32 integer,
    ITASN1_INTEGER **ppDerInteger)
{
	ITCAST_UINT8 *pData,*pMidSite;
	ITCAST_UINT32 *pLength;
	ITASN1_INTEGER *pInteger = NULL;
    ITCAST_UINT8 *pMidUint8 = NULL,cTag = ITCAST_DER_INTEGER;
    int iResult;
	
	//����pData��
	iResult = DER_ItAsn1_Low_IntToChar(integer,&pData,&pLength);
    if (iResult != ITDER_NoErr)
	{
		ITDER_LOG(__FILE__, __LINE__,LogLevel[4], iResult,"func DER_ItAsn1_Low_IntToChar() err != ITDER_NoErr");
		return  iResult;	
	}
	DER_ITASN1_LOW_CREATEUINT8(pMidUint8,*pLength+2);
	pMidSite = pMidUint8;
	*(pMidUint8++) = cTag;
	*(pMidUint8++) = (ITCAST_UINT8)*pLength;
	memcpy(pMidUint8,pData,*pLength);
	//�γ�ITASN1_INTEGER�ṹ
	DER_CREATE_LOW_ITCAST_ANYBUF(pInteger);
	DER_ITASN1_LOW_CHECKERR(iResult,ITDER_MemoryErr);
	pInteger ->pData = pMidSite;
	pInteger ->unusedBits = 0;
	pInteger ->dataType = DER_ITASN1_LOW_IDENTIFIER(cTag);
	pInteger ->memoryType = DER_ITASN1_MEMORY_MALLOC;
	pInteger ->dataLen = *pLength + 2; 
	pInteger ->next = NULL;
	pInteger ->prev = NULL;
	//���
    *ppDerInteger = pInteger;
	DER_ITCAST_Free(pData);
	DER_ITCAST_Free(pLength);
    //�м��������
	pMidSite = NULL;
	pInteger = NULL;
    pMidUint8 = NULL;
	
    return 0;
}

//DER������������
ITCAST_INT
DER_ItAsn1_ReadInteger(
    ITASN1_INTEGER *pDerInteger,
    ITCAST_UINT32 *pInteger)
{
    ITCAST_UINT8 *pMidData = NULL;
	ITCAST_UINT32 *pTag = NULL;
	ITCAST_UINT32 lMidLength,*pMidSize = NULL,*pMidSizeOf = NULL;
	int iResult;
    //����
	pMidData = pDerInteger ->pData;
    iResult = DER_ItAsn1_Low_GetTagInfo(&pMidData,&pTag,&pMidSize);
	if (iResult != ITDER_NoErr)
	{
		ITDER_LOG(__FILE__, __LINE__,LogLevel[4], iResult,"func DER_ItAsn1_ReadInteger() err check iResult != ITDER_NoErr");
		return  iResult;	
	}
	DER_ITCAST_Free(pTag);
	DER_ITCAST_Free(pMidSize);
	iResult = DER_ItAsn1_Low_GetLengthInfo(&pMidData,&pMidSize,&pMidSizeOf);
	if (iResult != ITDER_NoErr)
	{
		ITDER_LOG(__FILE__, __LINE__,LogLevel[4], iResult,"func DER_ItAsn1_Low_GetLengthInfo() err");
		return  iResult;	
	}
	lMidLength = *pMidSize;
	DER_ITCAST_Free(pMidSize);
	DER_ITCAST_Free(pMidSizeOf);
	iResult = DER_ItAsn1_Low_CharToInt(pMidData,lMidLength,&pMidSize);
	if (iResult != ITDER_NoErr)
	{
		ITDER_LOG(__FILE__, __LINE__,LogLevel[4], iResult,"func DER_ItAsn1_Low_CharToInt() err");
		return  iResult;	
	}
	//���
	*pInteger = *pMidSize;
	DER_ITCAST_Free(pMidSize);
    //�м��������
	pMidData = NULL;
	
    return 0;
}

//DER����BitString��������
ITCAST_INT
DER_ItAsn1_WriteBitString(
    ITASN1_BITSTRING *pBitString,
    ITASN1_BITSTRING **ppDerBitString)
{
	ITASN1_BITSTRING *pMidBitString = NULL;
	ITCAST_UINT8 *pMidData = NULL,*pMidValue = NULL;
	ITCAST_UINT8 cTag = ITCAST_DER_BITSTRING;
	int iResult;
	
    //����
	iResult = DER_ItAsn1_Low_WriteTagAndLength(pBitString,cTag,&pMidBitString,&pMidValue);
    if (iResult != ITDER_NoErr)
	{
		ITDER_LOG(__FILE__, __LINE__,LogLevel[4], iResult,"func DER_ItAsn1_Low_WriteTagAndLength() err");
		return  iResult;	
	}
	
	pMidData = pBitString ->pData;
	memcpy(pMidValue,pMidData,pBitString ->dataLen);
	//���
	*ppDerBitString = pMidBitString;
    //�м��������
	pMidBitString = NULL;
	pMidData = NULL;
	pMidValue = NULL;
	
    return 0;
}

//DER����BitString��������
ITCAST_INT
DER_ItAsn1_ReadBitString(
    ITASN1_BITSTRING *pDerBitString,
    ITASN1_BITSTRING **ppBitString)
{
	ITASN1_BITSTRING *pMidBitString = NULL;
	ITCAST_UINT8 *pMidData = NULL,*pMidValue = NULL;
	int iResult;
	
	//����
	pMidData = pDerBitString ->pData;
    iResult = DER_ItAsn1_Low_ReadTagAndLength(pDerBitString,&pMidData,&pMidBitString,&pMidValue);
	if (iResult != ITDER_NoErr)
	{
		ITDER_LOG(__FILE__, __LINE__,LogLevel[4], iResult,"func DER_ItAsn1_Low_ReadTagAndLength() err");
		return iResult; 
	}
	memcpy(pMidValue,pMidData,pMidBitString ->dataLen);
    //���
	*ppBitString = pMidBitString;
    //�м��������
	pMidBitString = NULL;
	pMidData = NULL;
	pMidValue = NULL;
	
    return 0;	
}


//DER����CharString��������
ITCAST_INT
DER_ItAsn1_WriteCharString(
    ITCAST_ANYBUF *pCharString,
    ITCAST_ANYBUF **ppDerCharString)
{
	ITCAST_ANYBUF *pMidCharString = NULL;
	ITCAST_UINT8 *pMidData = NULL,*pMidValue = NULL;
	ITCAST_UINT8 cTag = ITCAST_DER_ID_STRING_PRINTABLE; 
	int iResult;
	
	//����
	iResult = DER_ItAsn1_Low_WriteTagAndLength(pCharString,cTag,&pMidCharString,&pMidValue);
    if (iResult != ITDER_NoErr)
	{
		ITDER_LOG(__FILE__, __LINE__,LogLevel[4], iResult,"func DER_ItAsn1_Low_WriteTagAndLength() err");
		return  iResult;	
	}
	
	pMidData = pCharString ->pData;
	memcpy(pMidValue,pMidData,pCharString ->dataLen);
	//���
	*ppDerCharString =  pMidCharString;
    //�м��������
	pMidCharString = NULL;
	pMidData = NULL;
	pMidValue = NULL;

	return 0;
}

//DER����PrintableString��������
ITCAST_INT
DER_ItAsn1_ReadCharString(
    ITCAST_ANYBUF *pDerCharString,
    ITCAST_ANYBUF **ppCharString)
{
	ITCAST_ANYBUF *pMidCharString = NULL;
	ITCAST_UINT8 *pMidData = NULL,*pMidValue = NULL;
	int iResult;
	
	//����
	pMidData = pDerCharString ->pData;
	iResult = DER_ItAsn1_Low_ReadTagAndLength(pDerCharString,&pMidData,&pMidCharString,&pMidValue);
	if (iResult != ITDER_NoErr)
	{
		ITDER_LOG(__FILE__, __LINE__,LogLevel[4], iResult,"func DER_ItAsn1_Low_ReadTagAndLength() err");
		return  iResult;	
	}
	
	memcpy(pMidValue,pMidData,pMidCharString ->dataLen);
    //���
	*ppCharString = pMidCharString;
    //�м��������
	pMidCharString = NULL;
	pMidData = NULL;
	pMidValue = NULL;

    return 0;
}

//DER����BmpString��������
ITCAST_INT
DER_ItAsn1_WriteBmpString(
    ITASN1_BMPSTRING *pBmpString,
    ITASN1_BMPSTRING **ppDerBmpString)
{
	ITCAST_ANYBUF *pMidBmpString = NULL;
	ITCAST_UINT8 *pMidData = NULL,*pMidValue = NULL;
	ITCAST_UINT8 cTag = ITCAST_DER_ID_STRING_BMP; 
	int iResult;
	
	//����
	iResult = DER_ItAsn1_Low_WriteTagAndLength(pBmpString,cTag,&pMidBmpString,&pMidValue);
    if (iResult != ITDER_NoErr)
	{
		ITDER_LOG(__FILE__, __LINE__,LogLevel[4], iResult,"func DER_ItAsn1_Low_WriteTagAndLength() err");
		return  iResult;	
	}
	
	pMidData = pBmpString ->pData;
	memcpy(pMidValue,pMidData,pBmpString ->dataLen);
	//���
	*ppDerBmpString =  pMidBmpString;
    //�м��������
	pMidBmpString = NULL;
	pMidData = NULL;
	pMidValue = NULL;

	return 0;
}

//DER����BmpString��������
ITCAST_INT
DER_ItAsn1_ReadBmpString(
    ITASN1_BMPSTRING *pDerBmpString,
    ITASN1_BMPSTRING **ppBmpString)
{
	ITCAST_ANYBUF *pMidBmpString = NULL;
	ITCAST_UINT8 *pMidData = NULL,*pMidValue = NULL;
	int iResult;
	
	//����
	pMidData = pDerBmpString ->pData;
	iResult = DER_ItAsn1_Low_ReadTagAndLength(pDerBmpString,&pMidData,&pMidBmpString,&pMidValue);
	if (iResult != ITDER_NoErr)
	{
		ITDER_LOG(__FILE__, __LINE__,LogLevel[4], iResult,"func DER_ItAsn1_Low_ReadTagAndLength() err");
		return  iResult;	
	}
	
	memcpy(pMidValue,pMidData,pMidBmpString ->dataLen);
    //���
	*ppBmpString = pMidBmpString;
    //�м��������
	pMidBmpString = NULL;
	pMidData = NULL;
	pMidValue = NULL;

    return 0;
}
//DER����PrintableString��������
ITCAST_INT
DER_ItAsn1_WritePrintableString(
    ITASN1_PRINTABLESTRING *pPrintString,
    ITASN1_PRINTABLESTRING **ppDerPrintString)
{
	int iResult;
	
	if (pPrintString->dataType == ITCAST_DER_STRING_BMP)
	{
		iResult = DER_ItAsn1_WriteBmpString(pPrintString,ppDerPrintString);
        if (iResult != ITDER_NoErr)
		{
			ITDER_LOG(__FILE__, __LINE__,LogLevel[4], iResult,"func DER_ItAsn1_WriteBmpString() err");
			return  iResult;	
		}
	
	}
	else
    {
		iResult = DER_ItAsn1_WriteCharString(pPrintString,ppDerPrintString);
        if (iResult != ITDER_NoErr)
		{
			ITDER_LOG(__FILE__, __LINE__,LogLevel[4], iResult,"func DER_ItAsn1_WriteCharString() err");
			return  iResult;	
		}
	}
	
	return 0;
}

//DER����PrintableString��������
ITCAST_INT
DER_ItAsn1_ReadPrintableString(
						  ITASN1_PRINTABLESTRING *pDerPrintString,
						  ITASN1_PRINTABLESTRING **ppPrintString)
{
	ITASN1_PRINTABLESTRING *pMidPrintString = NULL;
	ITCAST_UINT8 *pMidData = NULL,*pMidValue = NULL;
	int iResult;
	
	//����
	pMidData = pDerPrintString ->pData;
	iResult = DER_ItAsn1_Low_ReadTagAndLength(pDerPrintString,&pMidData,&pMidPrintString,&pMidValue);
	if (iResult != ITDER_NoErr)
	{
		ITDER_LOG(__FILE__, __LINE__,LogLevel[4], iResult,"func DER_ItAsn1_Low_ReadTagAndLength() err");
		return  iResult;	
	}
	memcpy(pMidValue,pMidData,pMidPrintString ->dataLen);
    //���
	*ppPrintString = pMidPrintString;
    //�м��������
	pMidPrintString = NULL;
	pMidData = NULL;
	pMidValue = NULL;

    return 0;
}


//DER����Sequence��������
ITCAST_INT
DER_ItAsn1_WriteSequence(
    ITASN1_SEQUENCE *pSequence,
    ITCAST_ANYBUF **ppDerSequence)
{
	ITASN1_SEQUENCE *pMidSequence = NULL,*pMidNext1 = NULL;
	ITCAST_UINT8 *pMidValue = NULL,*pMidSite;
	ITCAST_UINT32 lSizeOf = 0,i,lMidLength = 0;
	ITCAST_UINT8 cTag = ITCAST_DER_SEQUENCE;
	
	
    //����Sequence���ݳ���
    pMidNext1 = pSequence;
	while (pMidNext1!= NULL)
	{
		lMidLength +=  pMidNext1 ->dataLen;
		pMidNext1 = pMidNext1 ->next;
	}
	lSizeOf = DER_ItAsn1_Low_Count_LengthOfSize(lMidLength);
	DER_ITASN1_LOW_CREATEUINT8(pMidValue,lMidLength + 1 +lSizeOf);
	
	pMidSite = pMidValue;
    //дTagֵ
	*(pMidValue++) = cTag;
	//д����
	if (lSizeOf == 1)
		*(pMidValue++) = (ITCAST_UINT8)lMidLength;
	else
	{
		*(pMidValue++) = ITCAST_DER_FIRST_YES_ID_MASK | ((ITCAST_UINT8)(lSizeOf-1));
		for (i = lSizeOf - 1;i > 0 ;i--)
			*(pMidValue++) = (ITCAST_UINT8)(lMidLength >>8*(i - 1));
	}
	pMidNext1 = pSequence;
    //copy����
	while (pMidNext1 != NULL)
	{
		memcpy(pMidValue,pMidNext1 ->pData,pMidNext1 ->dataLen);
	
		pMidValue += pMidNext1 ->dataLen;
		pMidNext1 = pMidNext1 ->next;
	}
	//����ITCAST_ANYBUF�ṹ
	DER_CREATE_LOW_ITCAST_ANYBUF(pMidSequence);
	pMidSequence ->dataLen = lMidLength + 1 +lSizeOf;
	pMidSequence ->pData = pMidSite;
	pMidSequence ->dataType = DER_ITASN1_LOW_IDENTIFIER(cTag);
	//���    
	*ppDerSequence = pMidSequence;
    //�м��������
	pMidSequence = NULL;
	pMidNext1 = NULL;
	pMidValue = NULL;
	pMidSite = NULL;

    return 0;
}



void DER_DI_FreeAnybuf(ITCAST_ANYBUF  * pAnyBuf)
{
	ITCAST_ANYBUF * pTmp;
	pTmp = pAnyBuf;
	if(pAnyBuf == NULL)
	{
		return;
	}
	while(pAnyBuf->next)
	{
		pTmp = pAnyBuf ->next;
		if (pAnyBuf->pData)
		{
			DER_ITCAST_Free(pAnyBuf->pData);
		}
		DER_ITCAST_Free(pAnyBuf);
		pAnyBuf = pTmp;
	}
	if (pAnyBuf->pData)
	{
		DER_ITCAST_Free(pAnyBuf->pData);
	}
	DER_ITCAST_Free(pAnyBuf);
	return;
}


//DER����Sequence��������
ITCAST_INT
DER_ItAsn1_ReadSequence(
    ITCAST_ANYBUF *pDerSequence,
    ITASN1_SEQUENCE **ppSequence)
{
	ITASN1_SEQUENCE *pMidNext1 = NULL,*pMidNext2 = NULL,*pMidSequence = NULL;
	ITCAST_UINT8   *pMidData = NULL,*pMidItemData = NULL,*pMidValue = NULL;
	ITCAST_UINT32  lMidLength = 0,*pTagValue = NULL,*pLengthValue = NULL;
	ITCAST_UINT32  *pTagSize = NULL,*pLengthSize = NULL;
	int        lTotalLength = 0;
	int iResult;
	ITCAST_UINT8 firstTag = TRUE;
	int i = 0;
	
	//��ʼ��
	pMidData = pDerSequence ->pData;
	//���Tagֵ
	iResult = DER_ItAsn1_Low_GetTagInfo(&pMidData,&pTagValue,&pTagSize);
	if (iResult != ITDER_NoErr)
	{
		ITDER_LOG(__FILE__, __LINE__,LogLevel[4], iResult,"func DER_ItAsn1_Low_GetTagInfo() err");
		return  iResult;	
	}
	
	if ((ITCAST_UINT8)*pTagValue != ITCAST_DER_ID_SEQUENCE)
	{
		DER_ITCAST_Free(pTagValue);
		DER_ITCAST_Free(pTagSize);
		ITDER_LOG(__FILE__, __LINE__,LogLevel[4], ITDER_InvalidTagErr,"check pTagValue != ITCAST_DER_ID_SEQUENCE err");
	    return ITDER_InvalidTagErr;
	}
	//��ⳤ��ֵ
	iResult = DER_ItAsn1_Low_GetLengthInfo(&pMidData,&pLengthValue,&pLengthSize);
	if (iResult != ITDER_NoErr)
	{
		DER_ITCAST_Free(pTagValue);
		DER_ITCAST_Free(pTagSize);
		ITDER_LOG(__FILE__, __LINE__,LogLevel[4], iResult,"fun DER_ItAsn1_Low_GetLengthInfo() err");
	    return iResult;	
	}

	if (pDerSequence ->dataLen != *pTagSize + *pLengthSize + *pLengthValue)
	{
		DER_ITCAST_Free(pLengthValue);
		DER_ITCAST_Free(pLengthSize);	    
		DER_ITCAST_Free(pTagValue);
		DER_ITCAST_Free(pTagSize);
		ITDER_LOG(__FILE__, __LINE__,LogLevel[4], ITDER_LengthNotEqualErr,"fun check length  err");
	    return ITDER_LengthNotEqualErr;	
	}
    lTotalLength = *pLengthValue;
	DER_ITCAST_Free(pTagValue);
	DER_ITCAST_Free(pTagSize);
	DER_ITCAST_Free(pLengthValue);
	DER_ITCAST_Free(pLengthSize);
	//����Sequence��,ͷ������Ԫ��
	while (lTotalLength > 0)
	{
		pMidItemData = pMidData;
		iResult = DER_ItAsn1_Low_GetTagInfo(&pMidItemData,&pTagValue,&pTagSize);
		if (iResult != ITDER_NoErr)
		{
			DER_ITCAST_Free(pTagValue);
			DER_ITCAST_Free(pTagSize);
			ITDER_LOG(__FILE__, __LINE__,LogLevel[4], iResult,"fun DER_ItAsn1_Low_GetTagInfo() err");
			return iResult;	
		}
		iResult = DER_ItAsn1_Low_GetLengthInfo(&pMidItemData,&pLengthValue,&pLengthSize);
		if (iResult != ITDER_NoErr)
		{
			DER_ITCAST_Free(pLengthValue);
			DER_ITCAST_Free(pLengthSize);		
			DER_ITCAST_Free(pTagValue);
			DER_ITCAST_Free(pTagSize);
			ITDER_LOG(__FILE__, __LINE__,LogLevel[4], iResult,"fun DER_ItAsn1_Low_GetLengthInfo() err");
			return iResult;	
		}
		lMidLength = *pTagSize + *pLengthSize + *pLengthValue;
			
        DER_ITASN1_LOW_CREATEUINT8(pMidValue,lMidLength);
		
		if (pMidValue == NULL)
		{
			DER_ITCAST_Free(pLengthValue);
			DER_ITCAST_Free(pLengthSize);		
			DER_ITCAST_Free(pTagValue);
			DER_ITCAST_Free(pTagSize);
			ITDER_LOG(__FILE__, __LINE__,LogLevel[4], ITDER_MemoryErr,"pMidValue is null err");
			return ITDER_MemoryErr;	
		}
		
		
		memcpy(pMidValue,pMidData,lMidLength);
		if (firstTag)
		{
			firstTag = FALSE;
			DER_CREATE_LOW_ITCAST_ANYBUF(pMidSequence);
			pMidSequence->pData = pMidValue;
			pMidSequence->dataType = *pTagValue;
			pMidSequence->dataLen = lMidLength;
			pMidNext2 = pMidSequence;
			pMidNext2 ->prev = NULL;
			pMidNext2 ->next = NULL;
		}
		else
		{
			DER_CREATE_LOW_ITCAST_ANYBUF(pMidNext1);
			pMidNext1 ->pData = pMidValue;
			pMidNext1 ->dataType = *pTagValue;
			pMidNext1 ->dataLen = lMidLength;
			pMidNext1 ->prev = pMidNext2;
			pMidNext2 ->next = pMidNext1;
			pMidNext2 = pMidNext2 ->next;
			pMidNext2 ->next = NULL;
			
		}
		pMidData += lMidLength;
		lTotalLength -= lMidLength;
		DER_ITCAST_Free(pTagValue);
	    DER_ITCAST_Free(pTagSize);
	    DER_ITCAST_Free(pLengthValue);
    	DER_ITCAST_Free(pLengthSize);
		i++;
	}
	if (lTotalLength != 0)
	{
		ITDER_LOG(__FILE__, __LINE__,LogLevel[4], ITDER_LengthErr,"lTotalLength != 0 err");
		DER_DI_FreeAnybuf(pMidSequence);		
	    return ITDER_LengthErr;
	}
	//��Ϣ����
    *ppSequence = pMidSequence;
    //�м��������
	pMidNext1 = NULL;
	pMidNext2 = NULL;
	pMidSequence = NULL;
	pMidData = NULL;
	pMidItemData = NULL;
	pMidValue = NULL;
	
    return 0;
}

//DER����Null��������
ITCAST_INT
DER_ItAsn1_WriteNull(
    ITCAST_ANYBUF **ppDerNull)
{
	ITCAST_ANYBUF *pMidNull = NULL;
	ITCAST_UINT8 *pMidValue = NULL,*pMidSite,cTag = ITCAST_DER_NULL;
    
	//����
	DER_ITASN1_LOW_CREATEUINT8(pMidValue,2);
	pMidSite = pMidValue;
	*(pMidValue++)= cTag;
	*pMidValue = 0x0;
	//����ITCAST_ANYBUF���ͽṹ
	DER_CREATE_LOW_ITCAST_ANYBUF(pMidNull);
	pMidNull ->pData = pMidSite;
	pMidNull ->dataLen = 2;
	pMidNull ->dataType = DER_ITASN1_LOW_IDENTIFIER(cTag);
	//���
	*ppDerNull = pMidNull;
    //�м��������
	pMidNull = NULL;
	pMidValue = NULL;
	pMidSite = NULL;

    return 0;
}

//DER����Null��������
ITCAST_INT
DER_ItAsn1_ReadNull(	ITCAST_ANYBUF  * pDerNull,
				ITCAST_UINT8  * pInt)
{
    unsigned char  temp[2];
    unsigned char  derNULL[2];
	memcpy(temp,pDerNull->pData,1);
	memcpy(temp+1,pDerNull->pData+1,1);
    memset(derNULL,5,1);
	memset(derNULL+1,0,1);

	if(memcmp(temp,derNULL,2))
	{
	    * pInt =0;
		return 1; 
	}
	
	* pInt = 5;
	
    return 0;
}

ITCAST_INT
DER_ITCAST_FreeQueue(ITCAST_ANYBUF *pAnyBuf)
{
	ITCAST_ANYBUF * pTmp;
	pTmp = pAnyBuf;
	if(pAnyBuf== NULL)
	{
		return ITDER_NoErr;
	}
	while(pAnyBuf->next)
	{
		pTmp = pAnyBuf ->next;
		if(pAnyBuf->pData)
		{
			DER_ITCAST_Free(pAnyBuf->pData);
		}
		DER_ITCAST_Free(pAnyBuf);
		pAnyBuf = pTmp;
	}
	if(pAnyBuf ->pData)
	{
			DER_ITCAST_Free(pAnyBuf->pData);
	}
	DER_ITCAST_Free(pAnyBuf);
	return ITDER_NoErr;
}


ITCAST_INT
DER_ITCAST_String_To_AnyBuf(ITCAST_ANYBUF **pOriginBuf,
	unsigned char * strOrigin,
	int  strOriginLen)
{
	ITCAST_ANYBUF *pValueBuf;

	pValueBuf = malloc(sizeof(ITCAST_ANYBUF));
	if (pValueBuf == NULL)
	{
		ITDER_LOG(__FILE__, __LINE__,LogLevel[4], ITDER_MemoryErr,"malloc err");
		return ITDER_MemoryErr;
	}
	memset(pValueBuf,0,sizeof(ITCAST_ANYBUF));

	if(strOriginLen <= 0)
	{
		pValueBuf ->pData = NULL;
		strOriginLen = 0;
	}
	else
	{
		pValueBuf->pData =(unsigned char *)malloc(strOriginLen);
		if (pValueBuf->pData == NULL)
		{
			DER_ITCAST_Free(pValueBuf);
			ITDER_LOG(__FILE__, __LINE__,LogLevel[4], ITDER_LengthErr,"malloc err");
			return ITDER_MemoryErr;
		}	
		memcpy(pValueBuf->pData,strOrigin,strOriginLen); 
	}
	pValueBuf->dataLen =  strOriginLen;
	pValueBuf->dataType = ITCAST_DER_ID_STRING_PRINTABLE;
	pValueBuf->next = NULL;
 	pValueBuf->prev = NULL;
	pValueBuf->unusedBits = (strOriginLen % 8);
	pValueBuf->memoryType = 0;
	*pOriginBuf = pValueBuf; 
	return ITDER_NoErr;
}

#define  DER_INPUTDATA_ERR 106
//�Կ�ָ����б���
int WriteNullSequence(ITCAST_ANYBUF **pOutData)
{
	int				rv = 0;
	ITCAST_ANYBUF		*pTmp=NULL, *pHead=NULL;

	rv = DER_ItAsn1_WriteNull(&pTmp);
	if (rv !=0)
	{
		DER_ITCAST_FreeQueue(pTmp);
		ITDER_LOG(__FILE__, __LINE__,LogLevel[4], rv,"func DER_ItAsn1_WriteNull() err");
		return rv;
	}

	rv = DER_ItAsn1_WriteSequence(pTmp, &pHead);
	if (rv != 0)
	{
		DER_ITCAST_FreeQueue(pTmp);
		DER_ITCAST_FreeQueue(pHead);
		ITDER_LOG(__FILE__, __LINE__,LogLevel[4], rv,"func DER_ItAsn1_WriteSequence() err");
		return rv;
	}
	DER_ITCAST_FreeQueue(pTmp);
	if (pHead == NULL)
	{
		ITDER_LOG(__FILE__, __LINE__,LogLevel[4], -1," check (pHead == NULL) err");
		return -1;
	}
	*pOutData = pHead;
	return 0;
}

int EncodeUnsignedChar(unsigned char *pData, int dataLen, ITCAST_ANYBUF **outBuf)
{

	ITCAST_ANYBUF	*pHeadBuf=NULL, *pTmp=NULL;	
	int			rv;

	//����ֵ���Ϸ�
	if (pData==NULL && dataLen!=0)
	{
		
		return DER_INPUTDATA_ERR;		
	}
	//����ֵ���Ϸ�
	else if (pData!=NULL && dataLen==0)
	{
		return DER_INPUTDATA_ERR;
	}
	else if (pData==NULL && dataLen==0)
	{
		rv = DER_ITCAST_String_To_AnyBuf(&pTmp, NULL, 0);
		if (rv != 0)
		{
			DER_ITCAST_FreeQueue(pTmp);
			//DER_DAPR_DebugMessage(_DEBUG_FILE_,__FILE__,__LINE__,"unsigned char*����   error");

			return rv;
		}

		rv = DER_ItAsn1_WriteBitString(pTmp, &pHeadBuf);
		if (rv != 0)
		{
			DER_ITCAST_FreeQueue(pTmp);
			DER_ITCAST_FreeQueue(pHeadBuf);
			//DER_DAPR_DebugMessage(_DEBUG_FILE_,__FILE__,__LINE__,"unsigned char*����   error");
			return rv;
		}	

	}
	else
	{
		rv = DER_ITCAST_String_To_AnyBuf(&pTmp,	pData, dataLen);
		if(rv != 0)
		{
			DER_ITCAST_FreeQueue(pTmp);
			//DER_DAPR_DebugMessage(_DEBUG_FILE_,__FILE__,__LINE__,"unsigned char*����   error");
			return -1;
		}
		rv = DER_ItAsn1_WriteBitString(pTmp,&pHeadBuf);
		if (rv != 0)
		{
			DER_ITCAST_FreeQueue(pHeadBuf);
			DER_ITCAST_FreeQueue(pTmp);
			//DER_DAPR_DebugMessage(_DEBUG_FILE_,__FILE__,__LINE__,"unsigned char*����   error");

			return -1;
		}	
	}
	DER_ITCAST_FreeQueue(pTmp);

	*outBuf = pHeadBuf;

	return 0;
}

int DecodeUnsignedChar(ITCAST_ANYBUF *inBuf, unsigned char **Data, int *pDataLen)
{

	ITCAST_ANYBUF	*pTmp=NULL;	
	int			rv;
	
	rv = DER_ItAsn1_ReadBitString(inBuf, &pTmp);
	if (rv != 0)
	{
		DER_ITCAST_FreeQueue(pTmp);
		//DER_DAPR_DebugMessage(_DEBUG_FILE_,__FILE__,__LINE__,"DecodeUnsignedChar  error");
		return -1;		
	}

	if (pTmp->dataLen == 0)
	{
		DER_ITCAST_FreeQueue(pTmp);
		*Data		= NULL;
		*pDataLen	= 0;
		return 0;
	}

	*Data = malloc(pTmp->dataLen+1);
	if (*Data == NULL)
	{
		DER_ITCAST_FreeQueue(pTmp);
		//DER_DAPR_DebugMessage(_DEBUG_FILE_,__FILE__,__LINE__,"DecodeUnsignedChar_an Mallco *Data Malloc  error");
		return -1; 		
	}	
	memset(*Data, 0, pTmp->dataLen+1);
	memcpy(*Data, pTmp->pData, pTmp->dataLen);
	*pDataLen = pTmp->dataLen;

	DER_ITCAST_FreeQueue(pTmp);

	return 0;
}


int EncodeChar(char *pData, int dataLen, ITCAST_ANYBUF **outBuf)
{

	ITCAST_ANYBUF	*pHeadBuf=NULL, *pTmp=NULL;
	int			rv;

	//����ֵ���Ϸ�	
	if (pData==NULL && dataLen!=0)
	{
		return DER_INPUTDATA_ERR;		
	}
	//����ֵ���Ϸ�
	else if (pData!=NULL && dataLen==0)
	{
		return DER_INPUTDATA_ERR;
	}
	else if (pData==NULL && dataLen==0)
	{
		rv = DER_ITCAST_String_To_AnyBuf(&pTmp, NULL, 0);
		if (rv != 0)
		{
			DER_ITCAST_FreeQueue(pTmp);
			//DER_DAPR_DebugMessage(_DEBUG_FILE_,__FILE__,__LINE__,"unsigned char*����   error");

			return -1;
		}

		rv = DER_ItAsn1_WritePrintableString(pTmp, &pHeadBuf);
		if (rv != 0)
		{
			DER_ITCAST_FreeQueue(pTmp);
			DER_ITCAST_FreeQueue(pHeadBuf);
			//DER_DAPR_DebugMessage(_DEBUG_FILE_,__FILE__,__LINE__,"unsigned char*����   error");

			return -1;
		}	

	}
	else
	{
		rv = DER_ITCAST_String_To_AnyBuf(&pTmp,	(unsigned char*)pData, dataLen);
		if(rv != 0)
		{
			DER_ITCAST_FreeQueue(pTmp);
			//DER_DAPR_DebugMessage(_DEBUG_FILE_,__FILE__,__LINE__,"unsigned char*����   error");
			return -1;
		}
		rv = DER_ItAsn1_WritePrintableString(pTmp, &pHeadBuf);
		if (rv != 0)
		{
			DER_ITCAST_FreeQueue(pHeadBuf);
			DER_ITCAST_FreeQueue(pTmp);
			//DER_DAPR_DebugMessage(_DEBUG_FILE_,__FILE__,__LINE__,"unsigned char*����   error");

			return -1;
		}	

	}
	DER_ITCAST_FreeQueue(pTmp);

	*outBuf = pHeadBuf;

	return 0;
}

int DecodeChar(ITCAST_ANYBUF *inBuf, char **Data, int *pDataLen)
{
	ITCAST_ANYBUF	*pTmp=NULL;	
	int			rv;

	rv = DER_ItAsn1_ReadPrintableString(inBuf, &pTmp);
	if (rv != 0)
	{
		DER_ITCAST_FreeQueue(pTmp);
		//DER_DAPR_DebugMessage(_DEBUG_FILE_,__FILE__,__LINE__,"DecodeUnsignedChar  error");
		return -1;		
	}

	if (pTmp->dataLen == 0)
	{
		DER_ITCAST_FreeQueue(pTmp);
		*Data		= NULL;
		*pDataLen	= 0;
		return 0;
	}

	*Data = malloc(pTmp->dataLen+1);
	if (*Data == NULL)
	{
		DER_ITCAST_FreeQueue(pTmp);
		//DER_DAPR_DebugMessage(_DEBUG_FILE_,__FILE__,__LINE__,"DecodeChar_an Mallco *Data Malloc  error");
		return -1; 		
	}
	memset(*Data, 0, pTmp->dataLen+1);
	memcpy(*Data, pTmp->pData, pTmp->dataLen);
	*pDataLen = pTmp->dataLen;

	DER_ITCAST_FreeQueue(pTmp);

	return 0;
}




