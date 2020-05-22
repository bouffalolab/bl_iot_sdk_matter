#clear_vars
LOCAL_SRCS_FILE:=

MODULE_DIR:= Common
MODULE_OUT_DIR:= $(TARGET_OUT_PATH)/Common

COMMON_CFLAGS:= -DMBEDTLS_CONFIG_FILE='<mbedtls_bflb_config.h>'
COMMON_INCLUDE:=  -I $(MODULE_DIR)/inc \
                  -I $(MODULE_DIR)/libc/inc \
                  -I $(MODULE_DIR)/cipher_suite/inc \
                  -I $(MODULE_DIR)/mbedtls/include/mbedtls \
                  -I $(MODULE_DIR)/mbedtls/bflb_port/inc \
                  -I $(MODULE_DIR)/mbedtls/include \
                  -I $(MODULE_DIR)/partition \
                  -I $(MODULE_DIR)/ring_buffer \

#ifeq ($(BOOTROM),y)
#COMMON_INCLUDE += -I $(MODULE_DIR)/platform_print
#else
#COMMON_INCLUDE += -I $(MODULE_DIR)/sim_print
#endif
COMMON_INCLUDE += -I $(MODULE_DIR)/platform_print
                  

#include sub-moudules
include $(MODULE_DIR)/libc/libc.mk
include $(MODULE_DIR)/platform_print/platform_print.mk
include $(MODULE_DIR)/cipher_suite/cipher_suite.mk
include $(MODULE_DIR)/soft_crc/softcrc.mk
include $(MODULE_DIR)/mbedtls/mbedtls.mk
include $(MODULE_DIR)/ring_buffer/ring_buffer.mk
include $(MODULE_DIR)/partition/partition.mk

common_objs_target := $(libc_objs) $(platform_print_objs) $(cipher_suite_objs) $(softcrc_objs)\
                      $(mbedtls_objs) $(ring_buffer_objs) $(partition_objs)

base_objects += $(common_objs_target)

.PHONY: common
common: $(common_objs_target)
