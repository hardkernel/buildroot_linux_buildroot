################################################################################
#
# test tools
#
################################################################################
TEST_TOOLS_VERSION = 20170616
TEST_TOOLS_SITE = $(TOPDIR)/../vendor/amlogic/test_tools
TEST_TOOLS_SITE_METHOD = local

define TEST_TOOLS_INSTALL_TARGET_CMDS
	cp -rf  $(@D)/test_plan  ${TARGET_DIR}/
endef

$(eval $(generic-package))
