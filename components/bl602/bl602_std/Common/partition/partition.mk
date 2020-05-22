SUB_MODULE_DIR:= $(MODULE_DIR)/partition
SUB_MODULE_OUT_DIR:= $(MODULE_OUT_DIR)/partition

SUB_MODULE_SRC_DIR := $(SUB_MODULE_DIR)

partition_sources := partition.c

partition_objs := $(addprefix $(SUB_MODULE_OUT_DIR)/, $(subst .c,.o,$(partition_sources)))

$(SUB_MODULE_OUT_DIR)/%.o:$(SUB_MODULE_SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "cc $<"
	$(AT)$(CC) -c $(GLOBAL_CFLAGS) $(COMMON_CFLAGS) $(GLOBAL_INCLUDE) $(COMMON_INCLUDE) $< -o $@
