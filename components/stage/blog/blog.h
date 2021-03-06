
#ifndef __BLOG_H__
#define __BLOG_H__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>

#include <utils_log.h>

#include "blog_type.h"
#include "blog_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif
#if (CFG_COMPONENT_BLOG_ENABLE == 1)
#define ATTR_BLOG_CODE1(name)        __attribute__((used, section(".static_blogcomponent_code." #name)))
#define ATTR_BLOG_CODE2(name)        __attribute__((used, section(".static_blogfile_code." #name)))
#define ATTR_BLOG_CODE3(name)        __attribute__((used, section(".static_blogpri_code." #name)))

/* for hard debug level */
const static uint32_t BLOG_HARD_DECLARE_DISABLE __attribute__((used)) = 0;

/* component level */
#define _REFC_LEVEL(name)            _fsymc_level_##name
#define REFC_LEVEL(name)             _REFC_LEVEL(name)
#define _DEFC_LEVEL(name)            blog_level_t REFC_LEVEL(name)
#define DECLARE_C_LEVEL(name)        _DEFC_LEVEL(name)
/* component info */
#define _REFC_INFO(name)             _fsymc_info_##name
#define REFC_INFO(name)              _REFC_INFO(name)
#define _DEFC_INFO(name)             static const blog_info_t REFC_INFO(name) ATTR_BLOG_CODE1(name) = {\
                                         (blog_level_t *)(&REFC_LEVEL(name)), (char *)(#name)}
#define DECLARE_C_INFO(name)         _DEFC_INFO(name)
DECLARE_C_LEVEL(__COMPONENT_NAME_DEQUOTED__);
DECLARE_C_INFO(__COMPONENT_NAME_DEQUOTED__);

/* file level */
#define _REFF_LEVEL(name)            _fsymf_level_##name
#define REFF_LEVEL(name)             _REFF_LEVEL(name)
#define _DEFF_LEVEL(name)            blog_level_t REFF_LEVEL(name)
#define DECLARE_F_LEVEL(name)        _DEFF_LEVEL(name)
/* file info */
#define _REFF_INFO(name)             _fsymf_info_##name
#define REFF_INFO(name)              _REFF_INFO(name)
#define _DEFF_INFO(name, named)      const blog_info_t REFF_INFO(name) ATTR_BLOG_CODE2(name) = {\
                                        (blog_level_t *)(&REFF_LEVEL(name)), (char *)(#named)}
#define DECLARE_F_INFO(name, named) _DEFF_INFO(name, named)
DECLARE_F_LEVEL(__COMPONENT_FILE_NAME_DEQUOTED__);
DECLARE_F_INFO(__COMPONENT_FILE_NAME_DEQUOTED__, __COMPONENT_FILE_NAMED__);

/* pri level */
#define _REFP_LEVEL(name)            _fsymp_level_##name
#define REFP_LEVEL(name)             _REFP_LEVEL(name)
#define _DEFP_LEVEL(name)            blog_level_t REFP_LEVEL(name)
#define DECLARE_P_LEVEL(name)        _DEFP_LEVEL(name)
/* pri info */
#define _REFP_INFO(name)             _fsymp_info_##name
#define REFP_INFO(name)              _REFP_INFO(name)
#define _DEFP_INFO(name, prefix)     const blog_info_t REFP_INFO(name) ATTR_BLOG_CODE3(name) = {\
                                        (blog_level_t *)(&REFP_LEVEL(name)), (char *)(#prefix"."#name)}
#define DECLARE_P_INFO(name, prefix) _DEFP_INFO(name, prefix)

#define BLOG_DECLARE(name)           DECLARE_P_LEVEL(name);\
                                     DECLARE_P_INFO(name, __COMPONENT_FILE_NAMED__);

#define custom_cflog(lowlevel, N, M, ...) do {\
    if ( (lowlevel >= REFC_LEVEL(__COMPONENT_NAME_DEQUOTED__)) && \
         (lowlevel >= REFF_LEVEL(__COMPONENT_FILE_NAME_DEQUOTED__))\
       ) {\
            __blog_printf("[%10u][%s: %s:%4d] " M,\
            (xPortIsInsideInterrupt())?(xTaskGetTickCountFromISR()):(xTaskGetTickCount()),\
            N, __FILENAME__, __LINE__,\
            ##__VA_ARGS__);\
    }\
} while(0==1)

#define custom_plog(priname, lowlevel, N, M, ...) do {\
    if ( (lowlevel >= REFC_LEVEL(__COMPONENT_NAME_DEQUOTED__)) && \
         (lowlevel >= REFF_LEVEL(__COMPONENT_FILE_NAME_DEQUOTED__)) && \
         (lowlevel >= REFP_LEVEL(priname)) \
       ) {\
            __blog_printf("[%10u][%s: %s:%4d] " M,\
            (xPortIsInsideInterrupt())?(xTaskGetTickCountFromISR()):(xTaskGetTickCount()),\
            N, __FILENAME__, __LINE__,\
            ##__VA_ARGS__);\
    }\
} while(0==1)

#define custom_hexdumplog(name, lowlevel, logo, buf, size) do {\
    if ( (lowlevel >= REFC_LEVEL(__COMPONENT_NAME_DEQUOTED__)) && \
         (lowlevel >= REFF_LEVEL(__COMPONENT_FILE_NAME_DEQUOTED__))\
       ) {\
            __blog_printf("[%10u][%s: %s:%4d] %s:\r\n",\
            (xPortIsInsideInterrupt())?(xTaskGetTickCountFromISR()):(xTaskGetTickCount()),\
            logo, __FILENAME__, __LINE__,\
            name);\
            blog_hexdump_out(name, 16, buf, size);\
    }\
} while(0==1)

#define blog_debug(M, ...)              if (0 == BLOG_HARD_DECLARE_DISABLE) {\
                                            custom_cflog(BLOG_LEVEL_DEBUG,"DEBUG ", M, ##__VA_ARGS__);}                 // NULL
#define blog_info(M, ...)               if (0 == BLOG_HARD_DECLARE_DISABLE) {\
                                            custom_cflog(BLOG_LEVEL_INFO,"\x1b[32mINFO  \x1b[0m", M, ##__VA_ARGS__);}    // F_GREEN
#define blog_warn(M, ...)               if (0 == BLOG_HARD_DECLARE_DISABLE) {\
                                            custom_cflog(BLOG_LEVEL_WARN,"\x1b[33mWARN  \x1b[0m", M, ##__VA_ARGS__);}    // F_YELLOW
#define blog_error(M, ...)              if (0 == BLOG_HARD_DECLARE_DISABLE) {\
                                            custom_cflog(BLOG_LEVEL_ERROR,"\x1b[31mERROR \x1b[0m", M, ##__VA_ARGS__);}  // F_RED
#define blog_assert(assertion)          if (0 == (assertion)) {\
                                                __blog_printf("assert, %s:%d\r\n", __FILENAME__, __LINE__);\
                                                while(1);\
                                            }

#define blog_debug_user(name, M, ...)   if (0 == BLOG_HARD_DECLARE_DISABLE) {\
                                            custom_plog(name,BLOG_LEVEL_DEBUG, "DEBUG ", M, ##__VA_ARGS__);}
#define blog_info_user(name, M, ...)    if (0 == BLOG_HARD_DECLARE_DISABLE) {\
                                            custom_plog(name,BLOG_LEVEL_INFO, "\x1b[32mINFO  \x1b[0m", M, ##__VA_ARGS__);}
#define blog_warn_user(name, M, ...)    if (0 == BLOG_HARD_DECLARE_DISABLE) {\
                                            custom_plog(name,BLOG_LEVEL_WARN, "\x1b[33mWARN  \x1b[0m", M, ##__VA_ARGS__);}
#define blog_error_user(name, M, ...)   if (0 == BLOG_HARD_DECLARE_DISABLE) {\
                                            custom_plog(name,BLOG_LEVEL_ERROR, "\x1b[31mERROR \x1b[0m", M, ##__VA_ARGS__);}
#define blog_assert_user(name, M, ...)  if (0 == BLOG_HARD_DECLARE_DISABLE) {\
                                            custom_plog(name,BLOG_LEVEL_ASSERT, "\x1b[35mASSERT\x1b[0m", M, ##__VA_ARGS__);}

#define blog_debug_hexdump(name, buf, size)   if (0 == BLOG_HARD_DECLARE_DISABLE) {\
                                              custom_hexdumplog(name,BLOG_LEVEL_DEBUG, "DEBUG ", buf, size);}
#define blog_info_hexdump(name, buf, size)    if (0 == BLOG_HARD_DECLARE_DISABLE) {\
                                              custom_hexdumplog(name,BLOG_LEVEL_INFO, "\x1b[32mINFO  \x1b[0m", buf, size);}
#define blog_warn_hexdump(name, buf, size)    if (0 == BLOG_HARD_DECLARE_DISABLE) {\
                                              custom_hexdumplog(name,BLOG_LEVEL_WARN, "\x1b[33mWARN  \x1b[0m", buf, size);}
#define blog_error_hexdump(name, buf, size)   if (0 == BLOG_HARD_DECLARE_DISABLE) {\
                                              custom_hexdumplog(name,BLOG_LEVEL_ERROR, "\x1b[31mERROR \x1b[0m", buf, size);}
#define blog_assert_hexdump(name, buf, size)  if (0 == BLOG_HARD_DECLARE_DISABLE) {\
                                              custom_hexdumplog(name,BLOG_LEVEL_ASSERT, "\x1b[35mASSERT\x1b[0m", buf, size);}
#define blog_print          __blog_printf
#define blog_buf            log_buf//unsupport

#else

//#define BLOG_HARD_DECLARE_DISABLE 1
#define BLOG_DECLARE(name)

#define blog_debug(M, ...)
#define blog_info(M, ...)
#define blog_warn(M, ...)
#define blog_error(M, ...)
#define blog_assert(M, ...)

#define blog_debug_user(name, M, ...)
#define blog_info_user(name, M, ...)
#define blog_warn_user(name, M, ...)
#define blog_error_user(name, M, ...)
#define blog_assert_user(name, M, ...)

#define blog_debug_hexdump(name, buf, size)
#define blog_info_hexdump(name, buf, size)
#define blog_warn_hexdump(name, buf, size)
#define blog_error_hexdump(name, buf, size)
#define blog_assert_hexdump(name, buf, size)

#define blog_print(...)
#define blog_buf(...)

#endif

void blog_init(void);

void blog_hexdump_out(const char *name, uint8_t width, uint8_t *buf, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif

