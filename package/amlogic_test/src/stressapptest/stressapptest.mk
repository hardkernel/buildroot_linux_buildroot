################################################################################
#
# amlogic image packer
#
#################################################################################
STRESSAPPTEST_VERSION = 2015
STRESSAPPTEST_SITE = $(TOPDIR)/package/stressapptest/
STRESSAPPTEST_SITE_METHOD = local

test_str=$(subst arm,armv7a,$(BR2_TOOLCHAIN_EXTERNAL_PREFIX))

define STRESSAPPTEST_CONFIGURE_CMDS
	( echo $(test_str);\
	  cd $(@D); \
		./configure \
		--host=$(BR2_TOOLCHAIN_EXTERNAL_PREFIX) \
	)
endef
		#--build=$(test_str) \


	  #cd $(@D); rm -rf config.cache; \

#	        $(TARGET_CONFIGURE_OPTS) \
#		$(TARGET_CONFIGURE_ARGS) \

define STRESSAPPTEST_BUILD_CMDS
	$(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) 
endef

define STRESSAPPTEST_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 $(@D)/src/stressapptest $(TARGET_DIR)/usr/bin
endef

#define STRESSAPPTEST_INSTALL_TARGET_CMDS
#	$(INSTALL) -m 0755 $(@D)/auto_suspend $(TARGET_DIR)/usr/bin
#	$(INSTALL) -m 0755 $(@D)/stressapptest.sh $(TARGET_DIR)/usr/bin
#	$(INSTALL) -m 0755 $(@D)/auto_freq.sh $(TARGET_DIR)/usr/bin
#	$(INSTALL) -m 0755 $(@D)/auto_reboot.sh $(TARGET_DIR)/usr/bin
#	$(INSTALL) -m 0755 $(@D)/disk-test.sh $(TARGET_DIR)/usr/bin
#	$(INSTALL) -m 0755 $(@D)/wifi_test.sh $(TARGET_DIR)/usr/bin
#endef

$(eval $(generic-package))

#STRESSAPPTEST_VERSION = 0.11.0
#STRESSAPPTEST_SITE = $(TOPDIR)/package/stressapptest
#STRESSAPPTEST_SITE_METHOD = local
#
#STRESSAPPTEST_INSTALL_STAGING = YES
#STRESSAPPTEST_AUTORECONF = YES
##STRESSAPPTEST_DEPENDENCIES = gst-aml-plugins
#
#$(eval $(autotools-package))

