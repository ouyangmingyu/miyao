/*********************************************************
 *  des.h
 *  �û�ʹ��des�㷨ͷ�ļ�
 *	
 *********************************************************/
#ifndef _OPENDESS_H_
#define _OPENDESS_H_

#ifdef __cplusplus
extern "C" {
#endif

//ab\0defg

//�û�ʹ�õĺ��� des ����
int DesEnc(
		unsigned char *pInData,
		int            nInDataLen,
		unsigned char *pOutData,
		int           *pOutDataLen);

//�û�ʹ�ú��� des����
int DesDec(
	   unsigned char *pInData,
	   int            nInDataLen,
	   unsigned char *pOutData,
	   int           *pOutDataLen);

int myic_DES (
	unsigned char *pDesKey,
	int            nDesKeyLen,
	unsigned char *pInData,
	int            nInDataLen,
	unsigned char *pOutData,
	int           *pOutDataLen);

int myic_DESEncrypt(
	unsigned char *pDesKey,
	int            nDesKeyLen,
	unsigned char *pInData,
	int            nInDataLen,
	unsigned char *pOutData,
	int           *pOutDataLen);

/*
int AppEncInterface(char *clentid, char *serverid, unsigned char *plain, int *plen, unsigned char *cryptdata, int *cryLen);
int AppDecInterface(char *clentid, char *serverid, unsigned char *plain, int *plen, unsigned char *cryptdata, int *cryLen);
*/
	
#ifdef __cplusplus
}
#endif

#endif

