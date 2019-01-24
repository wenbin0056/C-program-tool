


#include <stdio.h>
#include "commution_common.h"


typedef struct
{
	unsigned char bStartRecvThread;
	
}COMMU_SLAVER_STATE_T;


static COMMU_SLAVER_STATE_T commSlaverState;

typedef enum
{
	USER_CMD_SENDDATA,
	USER_CMD_REQUIREDATA,

	USER_CMD_MAX,			
		
}USER_CMD_E;


#define RX_PACK_HEAD_LEN	sizeof(PACK_HEADER_T)
#define TX_PACK_HEAD_LEN	sizeof(PACK_HEADER_T);

#define RX_ACK_PACK_HEAD_LEN	sizeof(ARK_PACK_T)
#define TX_ACK_PACK_HEAD_LEN	sizeof(ARK_PACK_T);

//======================USER SAMPLE==============================
int USER_SAMPLE_FUN(APP_ID_E appId)
{
	int ret = 0;
	
	ret = COMMUTION_WB_init(appId);
	if(ret != 0)
	{
		printf("err %s:%d\n",__func__, __LINE__);
		return -1;
	}
	unsigned char tx_buff[128] = {0};
	unsigned int tx_len = 128;
	unsigned char rx_buff[128] = {0};
	unsigned int rx_len = 128;

	PRESENTATION_sendPack(appId,USER_CMD_SENDDATA, tx_buff, tx_len, rx_buff, rx_len);
	
	return ret;
}

//============================================================


static CommutionStructT  CommutionStruct;

int COMMUTION_WB_init(APP_ID_E appID)
{
	int ret = 0;
	if(CommutionStruct.bTempBuffInit == 0)
	{
		CommutionStruct.pRxTempBuff= malloc(MAX_TX_BUFF_SIZE);
		CommutionStruct.pTxTempBuff= malloc(MAX_TX_BUFF_SIZE);		
		if(!CommutionStruct.pRxTempBuff || !CommutionStruct.pTxTempBuff)
		{
			return -1;
		}
		CommutionStruct.RxTempBuffLen = MAX_TX_BUFF_SIZE;
		CommutionStruct.TxTempBuffLen = MAX_TX_BUFF_SIZE;		
		CommutionStruct.bTempBuffInit = 1; 
	}
	
	if(CommutionStruct.bInit[appID] == 1)
	{
		return 0;
	}

	if(appID > APP_ID_MAX)
	{
		printf("param err %s:%d\n",__func__, __LINE__);
		return -1;
	}

	CommutionStruct.pTxBuff[appID] = malloc(MAX_TX_BUFF_SIZE);
	if(!CommutionStruct.pTxBuff[appID)
	{
		printf("malloc err %s:%d\n",__func__, __LINE__);
		return -1;		
	}

	CommutionStruct.pRxBuff[appID] = malloc(MAX_RX_BUFF_SIZE);	
	if(!CommutionStruct.pTxBuff[appID)
	{
		printf("malloc err %s:%d\n",__func__, __LINE__);
		return -1;		
	}	

	CommutionStruct.TxBuffLen[appID] = 0;
	CommutionStruct.RxBuffLen[appID] = 0;
	CommutionStruct.CurrentLayer[appID] = COMMUTION_LAYER_USER;
	CommutionStruct.bInit[appID] = 1;	

	if(1 != commSlaverState.bStartRecvThread )
	{
		int ThreadId;
		ret = pthread_create((pthread_t*)&ThreadId, NULL, SlaveRecvFun, NULL);
		if (0 != ret)
		{
			printf("pthread_create rtsp_frame_task failed \n");
			goto ERR;
		} 		
	}

	
	return 0;
	
ERR:
	free(CommutionStruct.pTxBuff[appID]);
	free(CommutionStruct.pRxBuff[appID]);
	free(CommutionStruct.RxTempBuffLen);	
	free(CommutionStruct.TTempBuffLen);		
	
	return -1;
	
}

int COMMUTION_WB_deInit(APP_ID_E appID)
{
	if(CommutionStruct.bTempBuffInit == 1)
	{
		free(CommutionStruct.pRxTempBuff);
		free(CommutionStruct.pTxTempBuff);		

		CommutionStruct.bTempBuffInit = 0; 
	}
	
	if(CommutionStruct.bInit[appID] == 0)
	{
		return 0;
	}
	
	if(appID > APP_ID_MAX)
	{
		printf("param err %s:%d\n",__func__, __LINE__);
		return -1;
	}

	free(CommutionStruct.pTxBuff[appID]);
	free(CommutionStruct.pRxBuff[appID]);
	free(CommutionStruct.RxTempBuffLen);	
	free(CommutionStruct.TTempBuffLen);		
	CommutionStruct.bInit[appID] = 0;	

	return 0;
}

void * SlaveRecvFun(void *parg)
{
	int appID = 0;
	int ret = 0;
	
	while(1 == commSlaverState.bStartRecvThread)
	{
		for(appID = 0; appID < APP_ID_MAX; appID++)
		{
			if(1 == CommutionStruct.bInit[appID])
			{
				ret = PRESENTATION_tryPack(appID);
				if(ret > 0)
				{
					//infor
				}
			}
			else
			{
				continue;
			}
		}
	}
	return NULL;
}

//======================PRESENTATION LAYER=====================

int PRESENTATION_recvPack()
{
	
}


int PRESENTATION_sendPack(APP_ID_E appId, USER_CMD_E cmdID, unsigned char *SendBuff, unsigned int SbuffLen, unsigned char *RxBuff,unsigned int RbuffLen)
{
	
	if(0 == CommutionStruct.bInit[appID])
	{
		printf("param err %s:%d\n",__func__, __LINE__);
		return -1;		
	}

	CommutionStruct.packHeader[appId].AppID =appId;
	memcpy(CommutionStruct.pTxBuff, SendBuff, SbuffLen);
	CommutionStruct.TxBuffLen[appId] = SbuffLen;
	CommutionStruct.pRxBuff[appId] = RxBuff;
	CommutionStruct.RxBuffLen[appId] = RbuffLen;	
	
	SESSION_sendPack(appId);

	SESSION_recvPack();
	return 0;
}



//======================SESSION LAYER=====================


int SESSION_recvPack()
{
	return 0;	
}

int SESSION_sendPack(APP_ID_E appId)
{
	static unsigned short sessionID = 0;
	sessionID++;
	Transport_sendPack(appId, sessionID);
	return 0;
}




//======================TRANSPORT LAYER=====================

int Transport_sendPack(APP_ID_E appId, unsigned short sessionID)
{
	CommutionStruct.TxpackHeader[appId].SessionID =appId;
	
	DATA_LINK_sendFrame(appId);
	
	return 0;
}

int Transport_recvPack()
{
	return 0;	
}

//======================DATA LIN LAYER=====================
// send frame by frame, if slave respond err, send again.
// control bit flow 
int DATA_LINK_sendFrame(APP_ID_E appId)
{
	unsigned int pos = 0;
	char buff[RX_PACK_HEAD_LEN] = {0};
	
	memset(CommutionStruct.pTxTempBuff, 0, CommutionStruct.RxTempBuffLen);

	unsigned int ret = 0;
	

	CommutionStruct.pTxTempBuff[pos++] = BYTE0(CommutionStruct.TxpackHeader[appId].AppID);
	CommutionStruct.pTxTempBuff[pos++] = BYTE1(CommutionStruct.TxpackHeader[appId].AppID);	

	CommutionStruct.pTxTempBuff[pos++] = BYTE0(CommutionStruct.TxpackHeader[appId].SessionID);
	CommutionStruct.pTxTempBuff[pos++] = BYTE1(CommutionStruct.TxpackHeader[appId].SessionID);	
	
	Phy_send_data(CommutionStruct.pTxTempBuff, pos);
	Phy_send_data(CommutionStruct.pTxBuff[appId], CommutionStruct.TxBuffLen[appId]);	

	ret = DATA_LINK_recvAck(appId);
	if(ret != 0)
	{
		return -1;
	}
	
	return 0;	
}


int DATA_LINK_sendAck(APP_ID_E appId)
{
	Phy_send_data(CommutionStruct.TxAckPack[appId], TX_ACK_PACK_HEAD_LEN);
	
	return 0;	
}

int DATA_LINK_RecvFrame(APP_ID_E appId)
{
	//TODO
	
	return 0;
}

int DATA_LINK_recvAck(APP_ID_E appId)
{
	Phy_recv_data(CommutionStruct.RxAckPack[appId], RX_ACK_PACK_HEAD_LEN);
	
	return 0;	
}



//======================PHY LAYER=====================

int Phy_send_data(char *pbuff, unsigned int size)
{
	//todo
	
	return 0;	
}

int Phy_recv_data(char *pbuff, unsigned int size)
{
	// todo 
	
	return 0;	
}

