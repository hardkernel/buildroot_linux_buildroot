################################################################################
#
# xdriver_xf86-video-fbturbo -- video driver for framebuffer device
#
################################################################################

XDRIVER_XF86_VIDEO_MALI_VERSION = r3p0-04rel0
XDRIVER_XF86_VIDEO_MALI_SITE = git://github.com/linux-sunxi/xf86-video-mali.git
XDRIVER_XF86_VIDEO_MALI_DEPENDENCIES = xserver_xorg-server xproto_xf86driproto xproto_fontsproto xproto_randrproto xproto_renderproto xproto_videoproto xproto_xproto
XDRIVER_XF86_VIDEO_MALI_AUTORECONF = YES

define XDRIVER_XF86_VIDEO_MALI_CHANGE_MODE
	chmod +x $(@D)/configure
endef
XDRIVER_XF86_VIDEO_MALI_POST_PATCH_HOOKS += XDRIVER_XF86_VIDEO_MALI_CHANGE_MODE

$(eval $(autotools-package))
