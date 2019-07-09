#############################################################
#
# IPC Webui
#
#############################################################

IPC_WEBUI_VERSION = 1.0
IPC_WEBUI_SITE = $(TOPDIR)/../vendor/amlogic/ipc/ipc_webui
IPC_WEBUI_SITE_METHOD = local
IPC_WEBUI_DEPENDENCIES = nginx php nodejs
define IPC_WEBUI_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/var/www/ipc-webui

	$(INSTALL) -D -m 644 $(IPC_WEBUI_SITE)/default.config/nginx.conf $(TARGET_DIR)/etc/nginx/nginx.conf
	$(INSTALL) -D -m 755 $(IPC_WEBUI_SITE)/default.config/php.ini $(TARGET_DIR)/etc/php.ini
	$(INSTALL) -D -m 755 $(IPC_WEBUI_PKGDIR)/S49ipc_webui $(TARGET_DIR)/etc/init.d

	rsync -avz $(IPC_WEBUI_SITE)/ $(TARGET_DIR)/var/www/ipc-webui --exclude 'default.config' --exclude '.git'

endef

$(eval $(generic-package))

