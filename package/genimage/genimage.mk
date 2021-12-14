################################################################################
#
# genimage
#
################################################################################

GENIMAGE_VERSION = 15
GENIMAGE_SOURCE = genimage-$(GENIMAGE_VERSION).tar.xz
GENIMAGE_SITE = https://public.pengutronix.de/software/genimage
HOST_GENIMAGE_DEPENDENCIES = host-pkgconf host-libconfuse
GENIMAGE_LICENSE = GPL-2.0
GENIMAGE_LICENSE_FILES = COPYING

$(eval $(host-autotools-package))
