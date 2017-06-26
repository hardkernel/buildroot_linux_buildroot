#############################################################
#
# GConf
#
#############################################################

GCONF_MAJOR_VERSION = 3.2
GCONF_MINOR_VERSION = 0
GCONF_VERSION = $(GCONF_MAJOR_VERSION).$(GCONF_MINOR_VERSION)
GCONF_SOURCE = GConf-$(GCONF_VERSION).tar.xz
GCONF_SITE = http://ftp.gnome.org/pub/gnome/sources/GConf/$(GCONF_MAJOR_VERSION)
GCONF_DEPENDENCIES = dbus-glib libxml2 host-intltool
GCONF_INSTALL_STAGING = YES
GCONF_CONF_OPTS += --libexecdir=/usr/lib/GConf --disable-orbit

define GCONF_POST_INSTALL
 # Gconf needs this empty directory to store it's stuff
 $(INSTALL) -v -m755 -d $(TARGET_DIR)/etc/gconf/gconf.xml.system
endef

GCONF_POST_INSTALL_TARGET_HOOKS += GCONF_POST_INSTALL

$(eval $(autotools-package))

