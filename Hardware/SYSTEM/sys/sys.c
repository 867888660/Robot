#include "sys.h"


//THUMB指令支持汇编函数
//执行WFI指令
void WFI_SET(void)
{
	__ASM volatile("wfi");		  
}
//关闭所有中断
void INTX_DISABLE(void)
{		  
	__ASM volatile("cpsid i");
}
//开启所有中断
void INTX_ENABLE(void)
{
	__ASM volatile("cpsie i");		  
}
//设置栈顶地址
//addr:栈顶地址
void MSR_MSP(u32 addr) 
{
    __ASM volatile("MSR MSP, %0" : : "r" (addr));
    __ASM volatile("BX LR");
}
