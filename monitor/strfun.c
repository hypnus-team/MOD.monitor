#include "cpu.h"
#include "net.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "str.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <time.h>
#include <pthread.h>

#ifdef DEBUG
#define debug_msg(format,...)   \
    fprintf(stdout,format,##__VA_ARGS__)
#else
    #define debug_msg(format,...)
#endif   /* DEBUG */

pthread_t monitor_tid = (pthread_t) 0;   //后台线程
unsigned int monitor_cpu[288] = {0}; //cpu记录
unsigned int monitor_net_in[288] =  {0}; //网络 in 记录
unsigned int monitor_net_out[288] = {0}; //网络 out记录
int c_n = 0;

void * monitor_daemon(void * tmp){
	time_t timep;
    struct tm *p;
	while (1){
        time(&timep);
		p = localtime(&timep); /*取得当地时间*/
        c_n = p->tm_hour * 60/5 + p->tm_min/5;
        debug_msg ("monitor cpu start ...\n");
		get_cpu_info(&monitor_cpu[c_n]);
		get_net_info(&monitor_net_in[c_n],&monitor_net_out[c_n]);
		sleep(5 * 60);
    }
}

int init(char *buff)
{
	if (buff){	        
	    char tmp[] = {0x03,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x00};
        memcpy(buff,tmp,16);
		
		if (!monitor_tid){
			debug_msg ("start new thread....\n");
		    pthread_create(&monitor_tid,NULL,monitor_daemon,NULL); 
		    pthread_detach(monitor_tid);
		}
		return 0;
	}else{
	    return -1;
	}
}




/* The cJSON structure: */
typedef struct cJSON {
	struct cJSON *next,*prev;	/* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
	struct cJSON *child;		/* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */

	int type;					/* The type of the item, as above. */

	char *valuestring;			/* The item's string, if type==cJSON_String */
	int valueint;				/* The item's number, if type==cJSON_Number */
	double valuedouble;			/* The item's number, if type==cJSON_Number */

	char *string;				/* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
} cJSON;


#pragma pack(1)
typedef struct{
	uint32_t RHeaderSize;
	char     Tid[32];
	void *   RData;
	uint32_t RDataSize;
    uint32_t RStatus;
	void *   ExecCallFunc;
	void *   ResponseSender;
	void *   ResponseStreamSender;
	cJSON *  cJSON_GetObjectItem;
	cJSON *  lpJsonRoot;
}REQUEST_HEADER;
#pragma pack()


int notify(const uint32_t nflag){
    if (1 == nflag){ // exit
		if (monitor_tid){
			debug_msg ("start cancel pthread ... \n");
			pthread_cancel(monitor_tid);
			pthread_join(monitor_tid, 0);
			debug_msg ("start cancel pthread ... finished \n");
		}
    }
    return 0;
}

int exec(const char *buff){
	REQUEST_HEADER * Buf = (REQUEST_HEADER *)buff;

    debug_msg ("Mod.exec: request len: %i contents: %s  addr: %p \n",Buf->RDataSize,(char *)Buf->RData,Buf->RData);
	debug_msg ("Mod.exec: request ID: %.*s \n",32,Buf->Tid);
	debug_msg ("Mod.exec: ExecFunc: %p \n",Buf->ExecCallFunc);
	debug_msg ("Mod.exec: cJSON_GetObjectItem: %p \n",Buf->cJSON_GetObjectItem);
	debug_msg ("Mod.exec: lpJsonRoot: %p \n",Buf->lpJsonRoot);
	

	int (* responseSender) (REQUEST_HEADER * , int,char*); 
	responseSender = Buf->ResponseSender;
    	
	//int bufferSize = 1024 * 1024;
	//char *buffer   = 0;
    
	int i; 
    
	
	// 288 * 4 = 1152 byte ，每个监控项 设置2KB 内存
    int retBuffSize = 1024*2 * 3;
	char *retBuff = malloc(retBuffSize);
	char tmp[100] = {0};

	if (retBuff){
		memset (retBuff,0x00,retBuffSize);
		
		sprintf (retBuff,"$i=%d\x04",c_n);		
		
		//cpu usage
		strcat (retBuff,"$cpu=");
        for (i=c_n+1;i<288;i++ ){  
			//printf ("%d,",i);
			sprintf (tmp,"%d,",monitor_cpu[i]);
			strcat (retBuff,tmp);
        }

		//printf ("\n%d\n",c_n);

        for (i =0;i<=c_n ;i++ ){          
			//printf ("%d,",i);
			sprintf (tmp,"%d,",monitor_cpu[i]);
			strcat (retBuff,tmp);
        }
		strcat (retBuff,"\x04");

        //network in
        strcat (retBuff,"$netin=");
        for (i=c_n+1;i<288;i++ ){ 
			sprintf (tmp,"%u,",monitor_net_in[i]);
			strcat (retBuff,tmp);
        }
        for (i =0;i<=c_n ;i++ ){  
			sprintf (tmp,"%u,",monitor_net_in[i]);
			strcat (retBuff,tmp);
        }
		strcat (retBuff,"\x04");

		//network out
        strcat (retBuff,"$netout=");
        for (i=c_n+1;i<288;i++ ){ 
			sprintf (tmp,"%u,",monitor_net_out[i]);
			strcat (retBuff,tmp);
        }
        for (i =0;i<=c_n ;i++ ){ 
			sprintf (tmp,"%u,",monitor_net_out[i]);
			strcat (retBuff,tmp);
        }
		strcat (retBuff,"\x04");
        
        responseSender(Buf,strlen(retBuff),retBuff);

		free (retBuff);
	}else{
		responseSender(Buf,5,"$e=1\x04");
	}
	

	#if DEBUG
   
	for (i =0;i<288 ;i++ ){
          if (0 == i%12){
				printf("\n");
            }
		printf ("%d[%d] ,",monitor_net_in[i],monitor_net_out[i]);

       }
    #endif	

	return 0;
}