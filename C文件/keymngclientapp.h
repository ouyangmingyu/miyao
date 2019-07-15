

#include <iostream>

using namespace std;

#include <unistd.h>
#include <sys/types.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>


#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <pthread.h>

#include "keymnglog.h"
#include "keymngclientop.h"

/* 
#include "poolsocket.h"
#include "keymng_msg.h"
*/

class KeyMngClientApp
{
public:
	KeyMngClientApp();
	~KeyMngClientApp();
	
public:
	int setKeyMngClientOp(KeyMngClientOp *keyMngClientOp);
	int setMngClientInfo(MngClient_Info *mngClientInfo);
	int Usage();
	int init();
	int run();
	int exit();	
	
public:	
	KeyMngClientOp		*m_keyMngClientOp;
	MngClient_Info 		*m_MngClientInfo;
};