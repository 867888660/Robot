#include "malloc.h"	    

// 内存池(32字节对齐)
// 修改__align为标准C语法
u8 mem1base[MEM1_MAX_SIZE] __attribute__((aligned(32)));  // 内部SRAM内存池

// 移除不兼容的at属性，改为普通数组
u8 mem2base[MEM2_MAX_SIZE] __attribute__((aligned(32)));  // 外部SRAM内存池

// 内存管理表
u16 mem1mapbase[MEM1_ALLOC_TABLE_SIZE];  // 内部SRAM内存池MAP

// 移除不兼容的at属性
u16 mem2mapbase[MEM2_ALLOC_TABLE_SIZE];  // 外部SRAM内存池MAP

// 内存块大小
const u32 memtblsize[SRAMBANK]={MEM1_ALLOC_TABLE_SIZE,MEM2_ALLOC_TABLE_SIZE};			// 内存块大小
const u32 memblksize[SRAMBANK]={MEM1_BLOCK_SIZE,MEM2_BLOCK_SIZE};						// 内存块大小
const u32 memsize[SRAMBANK]={MEM1_MAX_SIZE,MEM2_MAX_SIZE};								// 内存池大小


// 内存管理设备结构体
struct _m_mallco_dev mallco_dev=
{
	my_mem_init,				// 内存初始化
	my_mem_perused,				// 内存使用率
	mem1base,mem2base,			// 内存池
	mem1mapbase,mem2mapbase,	// 内存状态
	0,0,  		 				// 内存未分配
};


/**************************************************
函数名：mymemcpy(void *des,void *src,u32 n)  
函数功能：内存复制
参数：
	*des：目标地址
	*src：源地址
	n：需要复制的内存长度（字节为单位）
返回值：
	无
***************************************************/
void mymemcpy(void *des,void *src,u32 n)  
{  
    u8 *xdes=des;
	u8 *xsrc=src; 
    while(n--)*xdes++=*xsrc++;  
}  

/**************************************************
函数名：mymemset(void *s,u8 c,u32 count)  
函数功能：内存设置
参数：
	*s：内存起始地址
	c：需要设置的值
	count：需要设置的内存长度（字节为单位）
返回值：
	无
***************************************************/
void mymemset(void *s,u8 c,u32 count)  
{  
    u8 *xs = s;  
    while(count--)*xs++=c;  
}

/**************************************************
函数名：my_mem_init(u8 memx)
函数功能：内存初始化 
参数：
	memx：内存池编号
返回值：
	无
***************************************************/
void my_mem_init(u8 memx)  
{  
  mymemset(mallco_dev.memmap[memx], 0,memtblsize[memx]*2);// 内存状态清零  
	mymemset(mallco_dev.membase[memx], 0,memsize[memx]);	// 内存块清零  
	mallco_dev.memrdy[memx]=1;								// 内存初始化OK  
} 

/**************************************************
函数名：my_mem_perused(u8 memx) 
函数功能：获取内存使用率
参数：
	memx：内存池编号
返回值：
	内存使用率(0~100)
***************************************************/
u8 my_mem_perused(u8 memx)  
{  
    u32 used=0;  
    u32 i;  
    for(i=0;i<memtblsize[memx];i++)  
    {  
        if(mallco_dev.memmap[memx][i])used++; 
    } 
    return (used*100)/(memtblsize[memx]);  
}  

/**************************************************
函数名：my_mem_malloc(u8 memx,u32 size)  
函数功能：内存分配（内部内存池）
参数：
	memx：内存池编号
	size：需要分配的内存大小（字节）
返回值：
	0XFFFFFFFF，表示分配失败；否则，返回分配内存的偏移地址
***************************************************/
u32 my_mem_malloc(u8 memx,u32 size)  
{  
    signed long offset=0;  
    u32 nmemb;	// 需要分配的内存块数量  
	u32 cmemb=0;// 当前连续空闲内存块数量
    u32 i;  
    if(!mallco_dev.memrdy[memx])mallco_dev.init(memx);// 未初始化，调用初始化函数 
    if(size==0)return 0XFFFFFFFF;// 需要分配0字节
    nmemb=size/memblksize[memx];  	// 计算需要分配的内存块数量
    if(size%memblksize[memx])nmemb++;  
    for(offset=memtblsize[memx]-1;offset>=0;offset--)// 从内存池末尾向前查找
    {     
		if(!mallco_dev.memmap[memx][offset])cmemb++;// 找到连续空闲内存块
		else cmemb=0;								// 连续空闲内存块中断
		if(cmemb==nmemb)							// 找到nmemb个连续空闲内存块
		{
            for(i=0;i<nmemb;i++)  					// 标记内存块
            {  
                mallco_dev.memmap[memx][offset+i]=nmemb;  
            }  
            return (offset*memblksize[memx]);// 返回分配内存的偏移地址  
		}
    }  
    return 0XFFFFFFFF;// 未找到足够的连续空闲内存块  
}  

/**************************************************
函数名：my_mem_free(u8 memx,u32 offset)  
函数功能：释放内存（内部内存池） 
参数：
	memx：内存池编号
	offset：内存偏移地址
返回值：
	0，释放成功；1，释放失败；2，偏移地址无效
***************************************************/
u8 my_mem_free(u8 memx,u32 offset)  
{  
    int i;  
    if(!mallco_dev.memrdy[memx])// 未初始化，调用初始化函数
	{
		mallco_dev.init(memx);    
        return 1;// 未初始化  
    }  
    if(offset<memsize[memx])// 偏移地址超出内存池范围
    {  
        int index=offset/memblksize[memx];			// 计算内存块索引  
        int nmemb=mallco_dev.memmap[memx][index];	// 获取内存块大小
        for(i=0;i<nmemb;i++)  						// 释放内存块
        {  
            mallco_dev.memmap[memx][index+i]=0;  
        }  
        return 0;  
    }else return 2;// 偏移地址无效  
}

/**************************************************
函数名：myfree(u8 memx,void *ptr)  
函数功能：释放内存（外部内存池） 
参数：
	memx：内存池编号
	ptr：内存起始地址 
返回值：
	无
***************************************************/
void myfree(u8 memx,void *ptr)  
{  
	u32 offset;   
	if(ptr==NULL)return;// 地址为0
 	offset=(u32)ptr-(u32)mallco_dev.membase[memx];     
    my_mem_free(memx,offset);	// 释放内存      
}

/**************************************************
函数名：*mymalloc(u8 memx,u32 size)  
函数功能：内存分配（外部内存池）
参数：
	memx：内存池编号
	size：需要分配的内存大小（字节）
返回值：
	成功，返回分配内存的起始地址；失败，返回NULL
***************************************************/
void *mymalloc(u8 memx,u32 size)  
{  
    u32 offset;   
	offset=my_mem_malloc(memx,size);  	   	 	   
    if(offset==0XFFFFFFFF)return NULL;  
    else return (void*)((u32)mallco_dev.membase[memx]+offset);  
} 

/**************************************************
函数名：*myrealloc(u8 memx,void *ptr,u32 size)
函数功能：重新分配内存（外部内存池）
参数：
	memx：内存池编号
	*ptr：内存起始地址
	size：需要分配的内存大小（字节）
返回值：
	成功，返回重新分配内存的起始地址；失败，返回NULL
***************************************************/
void *myrealloc(u8 memx,void *ptr,u32 size)  
{  
    u32 offset;    
    offset=my_mem_malloc(memx,size);   	
    if(offset==0XFFFFFFFF)return NULL;     
    else  
    {  									   
	    mymemcpy((void*)((u32)mallco_dev.membase[memx]+offset),ptr,size);	// 复制内存数据到新内存块   
        myfree(memx,ptr);  											  		// 释放旧内存块
        return (void*)((u32)mallco_dev.membase[memx]+offset);  				// 返回新内存块的起始地址
    }  
}

/**************************************************
函数名：Malloc_Text(void)
函数功能：内存管理测试
参数：
	无
返回值：
	无
***************************************************/
void Malloc_Text(void)
{   
	u8 *p=0;
	u8 paddr[18];			// 打印P地址
	u8 sramx=0;				// 默认内部SRAM
	
	p=mymalloc(sramx,2048);// 分配2K内存
	if(p!=NULL)sprintf((char*)p,"Memory Malloc Test 111\n");// 写入一些数据
	sprintf((char*)paddr,"P Addr:0X%08X\n",(u32)p);
	printf("SRAMIN USED: %s\n",p);
	printf("SRAMIN USED: %s\n",paddr);
	printf("SRAMIN USED: %d\n",(int)(my_mem_perused(SRAMIN)));
	myfree(sramx,p);// 释放内存
	p=0;			// 指向空
	delay_ms(1000);
	delay_ms(1000);
	
	p=mymalloc(sramx,2048);// 分配2K内存
	if(p!=NULL)sprintf((char*)p,"Memory Malloc Test 222\n");// 写入一些数据
	sprintf((char*)paddr,"P Addr:0X%08X\n",(u32)p);
	printf("SRAMIN USED: %s\n",p);
	printf("SRAMIN USED: %s\n",paddr);
	printf("SRAMIN USED: %d\n",my_mem_perused(SRAMIN));
	myfree(sramx,p);// 释放内存
	p=0;			// 指向空
	delay_ms(1000);
	delay_ms(1000);
}









