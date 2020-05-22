# Copyright (C) 2016-2017, BouffaloLab Ltd.
# All Rights Reserved.

# source files and objs output dir
LOCAL_SRCS_FILE:= GCC/bl70x_startup.c system_bl70x.c
STARTUP_MODULE_DIR:= $(MODULE_DIR)/CMSIS/Device/Bouffalo/$(DEVICE)/Startup/
STARTUP_MODULE_OUT_DIR:= $(TARGET_OUT_PATH)/startup

# CFLAGS and included dirs for startup
STARTUP_CFLAGS:=
STARTUP_INCLUDE:=   -I$(MODULE_DIR)/CMSIS/Device/Bouffalo/$(DEVICE)/Startup \
					-I$(MODULE_DIR)/CMSIS/Core/Include \
					-I$(MODULE_DIR)/CMSIS/Device/Bouffalo/$(DEVICE)/Peripherals \

startup_objs:= $(addprefix $(STARTUP_MODULE_OUT_DIR)/,$(subst .c,.o,$(LOCAL_SRCS_FILE))) 

$(STARTUP_MODULE_OUT_DIR)/%.o:$(STARTUP_MODULE_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "cc $<"
	$(AT)$(CC) -c $(GLOBAL_CFLAGS) $(STARTUP_CFLAGS) $(GLOBAL_INCLUDE) $(STARTUP_INCLUDE) $< -o $@

base_objects += $(startup_objs)

.PHONY: startup
startup: $(startup_objs)
	@echo  "startup_objs is $(startup_objs)"
