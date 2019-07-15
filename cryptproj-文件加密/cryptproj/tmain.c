
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "des.h"

/*
int main333()
{
	int		rv = 0;

	unsigned char *pInData = "1234567887654321";
	int            nInDataLen = strlen(pInData);

	unsigned char OutData[2048];
	int           OutDataLen = 0;

	unsigned char buf2[2048];//明文2
	int           buf2Len = 0;


	memset(buf2, 0, sizeof(buf2));
	memset(OutData, 0, sizeof(OutData));

	//用户使用的函数
	rv =  DesEnc(pInData,nInDataLen, OutData, &OutDataLen);  //des 
	if (rv != 0)
	{
		printf("func DesEnc() err.err[%d]", rv);
		return rv;
	}

	//用户使用函数des解密
	rv = DesDec(OutData,OutDataLen, buf2, &buf2Len);
	if (rv != 0)
	{
		printf("func DesDec() err.err[%d]", rv);
		return rv;
	}

	if (nInDataLen != buf2Len)
	{
		rv = -1;
		printf("解密后的明文和原始明文长度不一致\n");
		return rv;
	}
	else
	{
		printf("解密后的明文和原始明文长度一致\n");
	}

	if (memcmp(pInData, buf2, buf2Len)!=0)
	{
		rv = -1;
		printf("解密后的明文和原始明文内容不一致\n");
		return rv;
	}
	else
	{
		printf("解密后的明文和原始明文内容一致\n");
	}

	printf("加解密绿灯测试通过\n");

	getchar();

	return 0;
}
//把file1 copy 成 file2
int movefile()
{
	int		rv = 0;
	FILE	*fp1 = NULL;
	FILE	*fp2 = NULL;

	char plain[4096];
	int  plainlen = 4096;

	char cryptdata[4096];
	int  cryptdatalen = 4096;

	int writened = 0;

	char *filename1 = "c:/itcast/1.txt";
	char *filename2 = "c:/itcast/2.txt";

	fp1 = fopen(filename1, "rb");
	if (fp1 == NULL)
	{
		rv = -1;
		goto End;
	}

	//判断文件2是否存在，如果存在报错
	fp2 = fopen(filename2, "rb");
	if (fp2 != NULL)
	{
		rv = -1;
		goto End;
	}
	else
	{
		if ((fp2 = fopen((char *)filename2, "wb")) == NULL)
		{
			rv = -3;
			printf("文件打不开错误\n");
			goto End;
		}
	}

	//如果文件大于4k，循环读fp1，到fp2,
	while (!feof(fp1))
	{
		plainlen = fread(plain, 1, Max_LEN, fp1);
		if (feof(fp1))
		{
			break;
		}
		if (plainlen != Max_LEN)
		{
			rv = -3;
			printf("读文件失败\n");
			goto End;
		}

		//用户使用的函数--不需要打padding
		rv = DesEnc_raw(plain, plainlen, cryptdata, &cryptdatalen);
		if (rv != 0)
		{
			printf("写新文件发生错误\n");
			goto End;
		}

		//判断写磁盘是否异常（磁盘满）
		writened = fwrite(plain, 1, plainlen, fp2);
		if (writened != plainlen)
		{
			rv = -4;
			printf("写新文件发生错误\n");
			goto End;
		}
	}

	//用户使用的函数--需要打padding
	rv = DesEnc(plain, plainlen, cryptdata, &cryptdatalen);
	if (rv != 0)
	{
		printf("用户调用加密函数失败\n");
		goto End;
	}
	//处理小于4k的数据
	//判断写磁盘是否异常（磁盘满）
	writened = fwrite(plain, 1, plainlen, fp2);
	if (writened != plainlen)
	{
		rv = -4;
		printf("写新文件发生错误\n");
		goto End;
	}

End:
	if (fp1 != NULL)
	{
		fclose(fp1);
	}
	if (fp2 != NULL)
	{
		fclose(fp2);
	}
}
*/
#define  MAX_DATA_LEN 4096

int FileSymEnc(char *filename1, char *filename2)
{
	int rv = 0;
	FILE			*fp1 = NULL;
	FILE			*fp2 = NULL;
	unsigned char	plain[4096];
	int				plainlen = 0;

	unsigned char	CipherData[4096];
	int				CipherDataLen = 0;
	int				WrittenLen = 0;

	//打开文件
	if((fp1=fopen((char *)filename1,"rb")) == NULL)
	{
		rv = -1;
		goto End;
	}

	if((fp2=fopen((char *)filename2,"rb")) != NULL)
	{
		rv = -2;
		fclose(fp2);
		fp2 = NULL; //野指针
		printf("文件已存在错误\n");
		goto End;
	}
	else
	{
		if((fp2=fopen((char *)filename2,"wb")) == NULL)
		{
			rv = -3;
			printf("文件打不开错误\n");
			goto End;
		}
	}

	while(!feof(fp1))
	{
		plainlen = fread(plain, 1, MAX_DATA_LEN, fp1);
		if(feof(fp1)){
			break;      //跳出循环，处理从明文文件中读取的最后一块数据
		}
		if(MAX_DATA_LEN != plainlen)
		{
			rv = -3;
			goto End;
		}

		//数据加密
		rv = DesEnc_raw(plain, plainlen, CipherData, &CipherDataLen);
		if (rv != 0)
		{
			printf("func DesEnc_raw() err:%d", rv);
			goto End;
		}

		WrittenLen = fwrite(CipherData, 1, CipherDataLen, fp2);
		if(WrittenLen != CipherDataLen)
		{
			rv = -4;
			goto End;
		}
	}

	//处理最后一小块数据
	//memcpy(CipherData, plain, plainlen);
	//CipherDataLen = plainlen;
	rv = DesEnc(plain, plainlen, CipherData, &CipherDataLen);
	if (rv != 0)
	{
		printf("func DesEnc_raw() err:%d", rv);
		goto End;
	}

	WrittenLen = fwrite(CipherData,1,CipherDataLen,fp2);
	if(WrittenLen != CipherDataLen){
		rv = -5;
		printf("写密文数据失败\n");
		return rv;
	}

End:
	if (fp1!= NULL)
	{
		fclose(fp1);
	}
	if (fp2 != NULL)
	{
		fclose(fp2);
	}
	return rv;
}

int FileSymDec(char *filename1, char *filename2)
{
	int rv = 0;
	FILE			*fp1 = NULL;
	FILE			*fp2 = NULL;
	unsigned char	plain[4096];
	int				plainlen = 0;

	unsigned char	CipherData[4096];
	int				CipherDataLen = 0;
	int				WrittenLen = 0;

	//打开文件
	if((fp1=fopen((char *)filename1,"rb")) == NULL)
	{
		rv = -1;
		goto End;
	}

	if((fp2=fopen((char *)filename2,"rb")) != NULL)
	{
		rv = -2;
		fclose(fp2);
		fp2 = NULL; //野指针
		printf("文件已存在错误\n");
		goto End;
	}
	else
	{
		if((fp2=fopen((char *)filename2,"wb")) == NULL)
		{
			rv = -3;
			printf("文件打不开错误\n");
			goto End;
		}
	}

	while(!feof(fp1))
	{
		CipherDataLen = fread(CipherData,1,MAX_DATA_LEN,fp1);
		if(feof(fp1)){
			break;      //跳出循环，处理密文文件中读取的最后一块数据
		}
		if(MAX_DATA_LEN != CipherDataLen)
		{
			rv = -3;
			goto End;
		}

		//数据解密
		rv = DesDec_raw(CipherData,CipherDataLen, plain, &plainlen);
		if (rv != 0)
		{
			printf("func DesEnc_raw() err:%d", rv);
			goto End;
		}

		WrittenLen = fwrite(plain, 1, plainlen, fp2);
		if(WrittenLen != plainlen)
		{
			rv = -4;
			goto End;
		}
	}

	//处理最后一小块数据
	//memcpy(CipherData, plain, plainlen);
	//CipherDataLen = plainlen;
	rv = DesDec(CipherData, CipherDataLen, plain, &plainlen);
	if (rv != 0)
	{
		printf("func DesEnc_raw() err:%d", rv);
		goto End;
	}

	WrittenLen = fwrite(plain,1,plainlen,fp2);
	if(WrittenLen != plainlen)
	{
		rv = -5;
		printf("写明文数据失败\n");
		return rv;
	}

End:
	if (fp1!= NULL)
	{
		fclose(fp1);
	}
	if (fp2 != NULL)
	{
		fclose(fp2);
	}
	return rv;
	
}

#define Max_LEN 4096 

int main()
{
	int rv = 0;
	char *filename1 = "c:/itcast/1.txt";
	char *filename2 = "c:/itcast/2.txt";
	char *filename3 = "c:/itcast/3.txt";

	rv = FileSymEnc(filename1, filename2);
	if (rv != 0)
	{
		printf("func FileSymEnc err[%d]", rv);
		return rv;
	}

	rv = FileSymDec(filename2, filename3);
	if (rv != 0)
	{
		printf("func FileSymDec err[%d]", rv);
		return rv;
	}

	getchar();
	return 0;
}
