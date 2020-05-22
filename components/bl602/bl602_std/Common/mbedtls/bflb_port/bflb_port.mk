SUB_MODULE_DIR:= $(MBEDTLS_DIR)/bflb_port
SUB_MODULE_OUT_DIR:= $(MBEDTLS_OUT_DIR)

SUB_MODULE_SRC_DIR := $(SUB_MODULE_DIR)/src

bflb_port_sources := bflb_aes.c bflb_sha1.c bflb_sha256.c bflb_bignum.c

bflb_port_objs := $(addprefix $(SUB_MODULE_OUT_DIR)/, $(subst .c,.o,$(bflb_port_sources)))

$(SUB_MODULE_OUT_DIR)/%.o:$(SUB_MODULE_SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "cc $<"
	$(AT)$(CC) -c $(GLOBAL_CFLAGS) $(COMMON_CFLAGS) $(MBEDTLS_CFLAGS) $(GLOBAL_INCLUDE) $(COMMON_INCLUDE) $< -o $@
