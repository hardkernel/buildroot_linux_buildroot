#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include <assert.h>
#include <sys/poll.h>

#ifdef TINYALSA_LIB
#include <tinyalsa/asoundlib.h>
#else
#include <alsa/asoundlib.h>
#endif

#include "line_in_select_channel.h"

#define LOG_TAG "AudioChannelSwitch"

static char card[64] = "default";

#ifdef TINYALSA_LIB

int line_in_select_channel(){

	LOGI("------------Select channel----------------------\n");
	
	struct mixer *mixer; 
	struct mixer_ctl *ctrl_handle;
	
	mixer = mixer_open(0);// sound card = 0
	
	ctrl_handle = mixer_get_ctl_by_name(mixer, "Capture Volume");
	
	int channel =  mixer_ctl_get_num_values(ctrl_handle);

	for (int id = 0; id < channel ; id++)
		mixer_ctl_set_value(ctrl_handle, id, 53);

	ctrl_handle = mixer_get_ctl_by_name(mixer, "Capture Volume ZC Switch");
	
	channel =  mixer_ctl_get_num_values(ctrl_handle);

	for (int id = 0; id < channel ; id++)
		mixer_ctl_set_value(ctrl_handle, id, 1);

	ctrl_handle = mixer_get_ctl_by_name(mixer, "ADC PCM Capture Volume");
	
	channel =  mixer_ctl_get_num_values(ctrl_handle);

	for (int id = 0; id < channel ; id++)
		mixer_ctl_set_value(ctrl_handle, id, 1);
	
	ctrl_handle = mixer_get_ctl_by_name(mixer, "ADC Output Select");
	
	mixer_ctl_set_value(ctrl_handle, 0, 1);

	ctrl_handle = mixer_get_ctl_by_name(mixer, "Left Input Mixer Boost Switch");
	
	//mixer_ctl_set_enum_by_string(ctrl_handle, "On");
	mixer_ctl_set_value(ctrl_handle, 0, 1);
	
	ctrl_handle = mixer_get_ctl_by_name(mixer, "Left Boost Mixer LINPUT2 Switch");
	
	//mixer_ctl_set_enum_by_string(ctrl_handle, "On");
	mixer_ctl_set_value(ctrl_handle, 0, 1);
		
	return 0;
}

#else

int snd_ctl_set(char *control_name, int value) {
	int err;
	snd_hctl_t *handle;
	snd_hctl_elem_t *elem;
	snd_ctl_elem_id_t *id;
	snd_ctl_elem_info_t *info;
	snd_ctl_elem_value_t *control;

	snd_ctl_elem_value_alloca(&control);
	snd_ctl_elem_id_alloca(&id);

	// open card
	if ((err = snd_hctl_open(&handle, card, 0)) < 0) {
		LOGD("Control %s open error: %s\n", card, snd_strerror(err));
		return err;
	}

	// load all ctl
	if ((err = snd_hctl_load(handle)) < 0) {
		LOGD("Control %s local error: %s\n", card, snd_strerror(err));
		return err;
	}

	// loop all ctl
	for (elem = snd_hctl_first_elem(handle); elem; elem = snd_hctl_elem_next(elem)) {
		snd_hctl_elem_get_id(elem, id);
		// macth snd ctl
		if (!strcmp(snd_ctl_elem_id_get_name(id), control_name)) {
			snd_ctl_elem_value_set_id(control, id);
			snd_ctl_elem_value_set_enumerated(control, 0, value);
			// set ctl value
			if ((err = snd_hctl_elem_write(elem, control)) < 0) {
				LOGD("Control %s element write error: %s\n", card,snd_strerror(err));
				return -1;
			}
		}
	}

	// close card
	snd_hctl_close(handle);

	return 0;
}

#endif


