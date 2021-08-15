#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

ifeq ($(CONFIG_ENABLE_VFS_ROMFS),1)
CPPFLAGS += -DCONF_USER_ENABLE_VFS_ROMFS
endif

LINKER_SCRIPTS := sdiowifi_flash_rom.ld

COMPONENT_ADD_LDFLAGS += -L $(BL60X_SDK_PATH)/customer_app/bl602_demo_sdiowifi/bl602_demo_sdiowifi \
                         $(addprefix -T ,$(LINKER_SCRIPTS))
COMPONENT_ADD_LINKER_DEPS := $(addprefix ,$(LINKER_SCRIPTS))
