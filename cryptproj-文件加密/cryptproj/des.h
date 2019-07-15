/*********************************************************
 *  des.h
 *  用户使用des算法头文件
 *	
 *********************************************************/
#ifndef _OPENDESS_H_
#define _OPENDESS_H_

#ifdef __cplusplus
extern "C" {
#endif

//ab\0defg

//用户使用的函数 des 加密
int DesEnc(
		unsigned char *pInData,
		int            nInDataLen,
		unsigned char *pOutData,
		int           *pOutDataLen);

//用户使用函数 des解密
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

