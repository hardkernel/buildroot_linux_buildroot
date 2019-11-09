#############################################################
#
# amlogic speaker process tools for avs
#
#############################################################
AML_SPEAKER_PROCESS_VERSION = 20171026
AML_SPEAKER_PROCESS_SITE = $(AML_SPEAKER_PROCESS_PKGDIR)/src
AML_SPEAKER_PROCESS_SITE_METHOD = local
AML_SPEAKER_PROCESS_DEPENDENCIES = avs-sdk

AML_SPEAKER_PROCESS_CFLAGS = \
    "-I$(@D)/../avs-sdk/KWD/DSP/include/ \
     -I$(STAGING_DIR)/usr/include/"
AML_SPEAKER_PROCESS_LDFLAGS = "-L$(@D)/../avs-sdk/KWD/DSP/lib/ -lAWELib"

define AML_SPEAKER_PROCESS_BUILD_CMDS
    $(MAKE) CC=$(TARGET_CXX) CFLAGS=$(AML_SPEAKER_PROCESS_CFLAGS) LDFLAGS=$(AML_SPEAKER_PROCESS_LDFLAGS) -C $(@D) all
endef

define AML_SPEAKER_PROCESS_INSTALL_TARGET_CMDS
    $(MAKE) -C $(@D) install
endef

$(eval $(generic-package))
