################################################################################
#
# amlogic 8189es driver
#
################################################################################

RTK8189ES_VERSION = $(call qstrip,$(BR2_PACKAGE_RTK8189ES_GIT_VERSION))
RTK8189ES_SITE = $(call qstrip,$(BR2_PACKAGE_RTK8189ES_GIT_REPO_URL))
$(eval $(generic-package))
