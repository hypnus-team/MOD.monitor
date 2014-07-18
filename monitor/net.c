#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>

#include "net.h"

#ifdef DEBUG
#define debug_msg(format,...)   \
    fprintf(stdout,format,##__VA_ARGS__)
#else
    #define debug_msg(format,...)
#endif   /* DEBUG */

double prev_in  = 0; // 保存上一个单位
double prev_out = 0; // 保存上一个单位

int get_net_info(unsigned int * lpInSave,unsigned int *lpOutSave){
        FILE *fp;
        char buf[228];
        char netcard[15];
		uint32_t * lpnetcard = (uint32_t *)&netcard;
		double tmp_input_bytes,tmp_output_bytes,tmp_output_packages;
        double input_bytes = 0;
		double output_bytes = 0;

		* lpInSave  = 0;
		* lpOutSave = 0;
		
		fp = fopen("/proc/net/dev","r");
        if(fp == NULL){
			return 0;
		}
                
		fgets(buf,sizeof(buf),fp);

		fgets(buf,sizeof(buf),fp);
		
		while (fgets(buf,sizeof(buf),fp)){

			debug_msg("buf=%s",buf);

			tmp_input_bytes = 0;
			tmp_output_bytes = 0;
			tmp_output_packages = 0;
            
			//sscanf注意:有特殊情况 eth:xxxx 连在一起，无空格
            sscanf(buf,"%*[ ]%[^:]:%lf%*f%*f%*f%*f%*f%*f%*f%lf%*f%*f%*f%*f%*f%*f%*f",netcard,&tmp_input_bytes,&tmp_output_bytes);
            
            if (((*lpnetcard) & 0x00FFFFFF) == 0x00006F6C){ //ignore lo:
				debug_msg ("\n ignore ! \n");
			}else{
				debug_msg ("\n [%s] input: %lf , output: %lf \n",netcard,tmp_input_bytes,tmp_output_bytes);
				input_bytes  += tmp_input_bytes;
				output_bytes += tmp_output_bytes;
			}			
		}
		fclose(fp);

		debug_msg ("\n current NetMonitor : input: %lf , output: %lf \n",input_bytes,output_bytes);
		debug_msg ("\n prev NetMonitor : input: %lf , output: %lf \n",prev_in,prev_out);

        if (0 == prev_in){

        }else{
		    * lpInSave  = (int)(input_bytes  - prev_in)/(5*60);
			* lpOutSave = (int)(output_bytes - prev_out)/(5*60);
		}

        prev_in  = input_bytes; // 保存上一个单位
        prev_out = output_bytes;



        return 1;

}