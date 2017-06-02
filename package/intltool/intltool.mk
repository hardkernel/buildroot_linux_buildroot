################################################################################
#
# intltool
#
################################################################################

INTLTOOL_VERSION = 0.51.0
INTLTOOL_SITE = https://launchpad.net/intltool/trunk/$(INTLTOOL_VERSION)/+download
INTLTOOL_LICENSE = GPLv2+
INTLTOOL_LICENSE_FILES = COPYING

HOST_INTLTOOL_DEPENDENCIES = host-gettext host-libxml-parser-perl

ifeq ($(BR2_NEEDS_GETTEXT),y)
INTLTOOL_DEPENDENCIES += gettext
endif
ifeq ($(BR2_HOST_ONLY),y)
INTLTOOL_DEPENDENCIES += libxml-parser-perl
endif

$(eval $(host-autotools-package))
$(eval $(autotools-package))
