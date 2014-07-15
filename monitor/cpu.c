#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cpu.h"

#ifdef DEBUG
#define debug_msg(format,...)   \
    fprintf(stdout,format,##__VA_ARGS__)
#else
    #define debug_msg(format,...)
#endif   /* DEBUG */

double prev_all  = 0; // 保存上一个单位
double prev_idle = 0; // 保存上一个单位

int get_cpu_info(unsigned int * lpSave){
	FILE *fp;
	char buf[228];
	char cpu[15];
	double user,nice,sys,idle,iowait,irq,softirq;
	double all;
	float usage;

	* lpSave = 0;

	fp = fopen("/proc/stat","r");
	if(fp == NULL){
		//perror("fopen:");
		return 0;
	}

	fgets(buf,sizeof(buf),fp);

	debug_msg("buf=%s",buf);

	sscanf(buf,"%s%lf%lf%lf%lf%lf%lf%lf",cpu,&user,&nice,&sys,&idle,&iowait,&irq,&softirq);

    all = user+nice+sys+idle+iowait+irq+softirq;
    fclose(fp);


	if (0 == prev_all){
		
	}else{	
		usage = (all - prev_all - (idle - prev_idle)) / (all - prev_all) * 100;
		* lpSave = (int)usage;
	}

	prev_all = all;
	prev_idle = idle;   

    return 1;
}
