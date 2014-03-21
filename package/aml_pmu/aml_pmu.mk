################################################################################
#
# amlogic pmu driver
#
################################################################################

AML_PMU_VERSION = $(call qstrip,$(BR2_PACKAGE_AML_PMU_GIT_VERSION))
AML_PMU_SITE = $(call qstrip,$(BR2_PACKAGE_AML_PMU_GIT_REPO_URL))
$(eval $(generic-package))
