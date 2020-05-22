#ifndef __DEMO_H__
#define __DEMO_H__

#define CI_CASE_TABLE_STEP1 {"[uart_case]", "start"}
#define CI_CASE_TABLE_STEP2 {"[uart_case]", "send case"}
#define CI_CASE_TABLE_STEP3 {"[uart_case]", "recv case"}
#define CI_CASE_TABLE_STEP4 {"[uart_case]", "end"}

void ci_loop_proc(void);

#endif

