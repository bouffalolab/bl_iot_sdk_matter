#include "bl70x.h"
#include "bl70x_glb.h"
#include "bl70x_hbn.h"
#include "bl70x_timer.h"
#include "system_bl70x.h"

/*----------------------------------------------------------------------------
  Define clocks
 *----------------------------------------------------------------------------*/
#define  RC32M            ( 32000000UL )      /* Oscillator frequency */
#ifdef CPU_NP_CM0
	/* 32MHz */
	#define  SYSTEM_CLOCK     ( 32000000UL )
#endif
#ifdef CPU_AP_CM4
	/* 144MHz */
	#define  SYSTEM_CLOCK     ( 144000000UL )
#endif

/*----------------------------------------------------------------------------
  Vector Table
 *----------------------------------------------------------------------------*/
#define VECT_TAB_OFFSET  0x00 /*!< Vector Table base offset field.
                                   This value must be a multiple of 0x200. */

/*----------------------------------------------------------------------------
  System Core Clock Variable
 *----------------------------------------------------------------------------*/
uint32_t SystemCoreClock = SYSTEM_CLOCK;

/*----------------------------------------------------------------------------
  System Core Clock update function
 *----------------------------------------------------------------------------*/
void SystemCoreClockUpdate (void)
{
	/*just for stub */
	SystemCoreClock = SYSTEM_CLOCK;
   
#ifdef CPU_NP_CM0
	/* 32MHz */
    SystemCoreClock = (32000000UL);
#endif
#ifdef CPU_AP_CM4
	/* 144MHz */
    SystemCoreClock = (144000000UL);
#endif

}

/*----------------------------------------------------------------------------
  System initialization function
 *----------------------------------------------------------------------------*/

void SystemInit (void)
{
#ifdef BL70X_BOOTROM
    /* NP boot log Flag */
    uint32_t *p= (uint32_t *)(BL70X_SRAM_END-24);    
    *p=0x5A5AA5A5;
    /* Disable Watchdog */
    WDT_Disable_All();
    HBN_Clear_RTC_INT();
    /*Power up soc 11 power domain,TODO: This should be optional */
    //AON_Power_On_SOC_11();
    /* Power up flash power*/
    AON_Power_On_IO_18();
#endif
    
#if defined (__FPU_USED) && (__FPU_USED == 1U)
    SCB->CPACR |= ((3U << 10U*2U) |           /* set CP10 Full Access */
                 (3U << 11U*2U)  );         /* set CP11 Full Access */
#endif
    
#ifdef UNALIGNED_SUPPORT_DISABLE
    SCB->CCR |= SCB_CCR_UNALIGN_TRP_Msk;
#endif
	
#ifdef CPU_NP_CM0
//	/* power on all clock source */
//	GLB_Power_On_System_Clock(GLB_RC32M_CLK_32M);
//	
//	/* select and set system clock */
//	GLB_Set_NP_System_CLK(GLB_NP_SYS_CLK_RC32M);
//	GLB_Set_NP_System_CLK_Div(0);
#endif
#ifdef CPU_AP_CM4
	SCB->VTOR = 0x00400000;
//	/* power on all clock source */
//	GLB_Power_On_System_Clock(GLB_XTAL_CLK_32M);
//	
//	/* select and set system clock */
//	GLB_Set_AP_System_CLK(GLB_AP_SYS_CLK_DLL_144M);
//	GLB_Set_AP_System_CLK_Div(0);
#endif
	
	/* update SystemCoreClock value */
    SystemCoreClockUpdate();

    __enable_irq();
	
	/* set NVIC group */
#ifdef CPU_NP_CM0
	/* not available for Cortex-M0 */
#endif
#ifdef CPU_AP_CM4
	NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_3);
#endif

}
#ifdef CPU_AP_CM4
void System_NVIC_SetPriority(IRQn_Type IRQn, uint32_t PreemptPriority, uint32_t SubPriority)
{
  uint32_t prioritygroup = 0x00;

  prioritygroup = NVIC_GetPriorityGrouping();

  NVIC_SetPriority(IRQn, NVIC_EncodePriority(prioritygroup, PreemptPriority, SubPriority));
}
#endif
#ifdef CPU_NP_CM0
void System_NVIC_SetPriority(IRQn_Type IRQn, uint32_t PreemptPriority, uint32_t SubPriority)
{
  //uint32_t prioritygroup = 0x00;
  NVIC_SetPriority(IRQn, PreemptPriority);
}
#endif


void Systick_Stop()
{
	SysTick->CTRL &= ~(SysTick_CTRL_ENABLE_Msk);
}

void Systick_Start()
{
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}
