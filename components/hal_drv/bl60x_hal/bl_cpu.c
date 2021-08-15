#include "bl_cpu.h"

void bl_cpu_word_copy(uint32_t *src, uint32_t *dst, uint32_t words)
{
    
    
    __asm volatile(
        "                                       \r\n"
        "          MOVS   r3, r2, LSR #3        \r\n"
        "          BEQ    copywords             \r\n"
        "          PUSH   {r4-r11}              \r\n"
        "    octcopy:                           \r\n"
        "          LDM    r0!, {r4-r11}         \r\n"
        "          STM    r1!, {r4-r11}         \r\n"
        "          SUBS   r3, r3, #1            \r\n"
        "          BNE    octcopy               \r\n"
        "          POP    {r4-r11}              \r\n"
        "    copywords:                         \r\n"
        "          ANDS   r2, r2, #7            \r\n" //Number of odd words to copy
        "          BEQ    stop                  \r\n" //No words left to copy?
        "    wordcopy:                          \r\n"
        "          LDR    r3, [r0], #4          \r\n" //Load a word from the source and
        "          STR    r3, [r1], #4          \r\n" //store it to the destination
        "          SUBS   r2, r2, #1            \r\n" //Decrement the counter
        "          BNE    wordcopy              \r\n" //... copy more
        "    stop:                              \r\n"
        "          MOV    r0, #0x0              \r\n"
    );
}
