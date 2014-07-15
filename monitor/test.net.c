#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "net.h"

#ifdef DEBUG
#define debug_msg(format,...)   \
    fprintf(stdout,format,##__VA_ARGS__)
#else
    #define debug_msg(format,...)
#endif   /* DEBUG */

int main(int argc ,char *argv[]){
   unsigned int monitor_net_in  = 0; //ÍøÂç in ¼ÇÂ¼
   unsigned int monitor_net_out = 0; //ÍøÂç out¼ÇÂ¼
   while (1){
       get_net_info(&monitor_net_in,&monitor_net_out);
	   debug_msg("\n Monitor Result: in: %u  , out: %u   \n",monitor_net_in,monitor_net_out);
	   sleep (20);
   }

}
