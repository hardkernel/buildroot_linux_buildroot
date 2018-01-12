################################################################################
#
# web_ui_wifi
#
################################################################################

WEB_UI_WIFI_VERSION = 20170915
WEB_UI_WIFI_SITE_METHOD = local
WEB_UI_WIFI_SITE = ${TOPDIR}/package/web_ui_wifi/src

ifeq ($(BR2_aarch64),y)
	TMP_NAME = main64.cgi
else
	TMP_NAME = main32.cgi
endif


define WEB_UI_WIFI_INSTALL_TARGET_CMDS
	mkdir -p ${TARGET_DIR}/var/www
	cp $(@D)/Html/* ${TARGET_DIR}/var/www/
	cp $(@D)/cgi-bin/${TMP_NAME} ${TARGET_DIR}/var/www/cgi-bin/main.cgi
	cp -rf $(@D)/cgi-bin/scripts/* ${TARGET_DIR}/var/www/cgi-bin/
endef

$(eval $(generic-package))
