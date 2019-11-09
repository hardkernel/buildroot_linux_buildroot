#############################################################
#
# Amlogic Toolchain for Cobalt/Chromium
#
#############################################################
BROWSER_TOOLCHAIN_DEPOT_TOOLS_VERSION		= 29b7b99e-2018.08.02
BROWSER_TOOLCHAIN_DEPOT_TOOLS_SOURCE		= depot_tools-$(BROWSER_TOOLCHAIN_DEPOT_TOOLS_VERSION).tar.gz
BROWSER_TOOLCHAIN_DEPOT_TOOLS_SITE		= http://openlinux.amlogic.com:8000/download/GPL_code_release/ThirdParty

BROWSER_TOOLCHAIN_DEPOT_TOOLS_INSTALL_DIR = $(HOST_DIR)/usr/browser-toolchain/depot_tools-$(BROWSER_TOOLCHAIN_DEPOT_TOOLS_VERSION)

define BROWSER_TOOLCHAIN_DEPOT_TOOLS_INSTALL_TARGET_CMDS
	rm -rf $(BROWSER_TOOLCHAIN_DEPOT_TOOLS_INSTALL_DIR)/*
	mkdir -p $(BROWSER_TOOLCHAIN_DEPOT_TOOLS_INSTALL_DIR)
	mv $(@D)/* $(BROWSER_TOOLCHAIN_DEPOT_TOOLS_INSTALL_DIR)/
endef

BROWSER_DEPOT_TOOL_PATH=$(BROWSER_TOOLCHAIN_DEPOT_TOOLS_INSTALL_DIR)

$(eval $(generic-package))
