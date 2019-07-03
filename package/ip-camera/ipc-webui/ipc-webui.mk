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

	$(INSTALL) -D -m 644 $(IPC_WEBUI_SITE)/index.php $(TARGET_DIR)/var/www/ipc-webui
	$(INSTALL) -D -m 644 $(IPC_WEBUI_SITE)/check_database.php $(TARGET_DIR)/var/www/ipc-webui
	$(INSTALL) -D -m 644 $(IPC_WEBUI_SITE)/common.php $(TARGET_DIR)/var/www/ipc-webui
	$(INSTALL) -D -m 644 $(IPC_WEBUI_SITE)/view-stream.php $(TARGET_DIR)/var/www/ipc-webui
	$(INSTALL) -D -m 644 $(IPC_WEBUI_SITE)/sqlite_create.sql $(TARGET_DIR)/var/www/ipc-webui
	$(INSTALL) -D -m 644 $(IPC_WEBUI_SITE)/default.config/nginx.conf $(TARGET_DIR)/etc/nginx/nginx.conf
	$(INSTALL) -D -m 755 $(IPC_WEBUI_SITE)/default.config/php.ini $(TARGET_DIR)/etc/php.ini
	$(INSTALL) -D -m 755 $(IPC_WEBUI_PKGDIR)/S49ipc_webui $(TARGET_DIR)/etc/init.d


	cp -af $(IPC_WEBUI_SITE)/facetable $(TARGET_DIR)/var/www/ipc-webui
	cp -af $(IPC_WEBUI_SITE)/usertable $(TARGET_DIR)/var/www/ipc-webui
	cp -af $(IPC_WEBUI_SITE)/upload    $(TARGET_DIR)/var/www/ipc-webui
	cp -af $(IPC_WEBUI_SITE)/layui $(TARGET_DIR)/var/www/ipc-webui
	cp -af $(IPC_WEBUI_SITE)/jsmpeg $(TARGET_DIR)/var/www/ipc-webui
endef

$(eval $(generic-package))

