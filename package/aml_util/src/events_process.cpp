/*
 * Copyright (C) Amlogic
 *
 * author: peipeng.zhao@amlogic.com
 */

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <linux/input.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include "events.h"
#include "events_process.h"

#define WAIT_KEY_TIMEOUT_SEC    120
#define nullptr NULL
#define KEY_EVENT_TIME_INTERVAL 20

EventsProcess::KeyMapItem_t g_default_keymap[] = {
        { "select", KEY_POWER, {KEY_POWER,KEY_ENTER, KEY_BACK, -1, -1, -1} },
        { "down", KEY_VOLUMEDOWN, {KEY_VOLUMEDOWN, KEY_DOWN,KEY_PAGEDOWN, -1, -1, -1} },
        { "up", KEY_VOLUMEUP, {KEY_VOLUMEUP, KEY_UP, KEY_PAGEUP, -1, -1, -1} },
    };

EventsProcess:: CtrlInfo_t g_ctrlinfo[] = {
        { "select", KEY_ENTER },
        { "down", KEY_DOWN },
        { "up", KEY_UP },
    };

EventsProcess::EventsProcess()
        : key_queue_len(0),
          key_last_down(-1),
          key_long_press(false),
          key_down_count(0),
          consecutive_power_keys(0),
          last_key(-1),
          has_power_key(false),
          has_up_key(false),
          has_down_key(false),
          num_keys(0),
          keys_map(NULL) {
    pthread_mutex_init(&key_queue_mutex, nullptr);
    pthread_cond_init(&key_queue_cond, nullptr);
    memset(key_pressed, 0, sizeof(key_pressed));
    memset(&last_queue_time, 0, sizeof(last_queue_time));
    load_key_map();
}

void EventsProcess::OnKeyDetected(int key_code) {
    if (key_code == KEY_POWER) {
        has_power_key = true;
    } else if (key_code == KEY_DOWN || key_code == KEY_VOLUMEDOWN) {
        has_down_key = true;
    } else if (key_code == KEY_UP || key_code == KEY_VOLUMEUP) {
        has_up_key = true;
    }
}

int EventsProcess::InputCallback(int fd, uint32_t epevents, void* data) {
    return reinterpret_cast<EventsProcess*>(data)->OnInputEvent(fd, epevents);
}

// Reads input events, handles special hot keys, and adds to the key queue.
static void* InputThreadLoop(void*) {
    while (true) {
        if (!ev_wait(-1)) {
            ev_dispatch();
        }
    }
    return nullptr;
}

void EventsProcess::Init() {
    ev_init(InputCallback, this);

    pthread_create(&input_thread_, nullptr, InputThreadLoop, nullptr);
}

int EventsProcess::OnInputEvent(int fd, uint32_t epevents) {
    struct input_event ev;

    if (ev_get_input(fd, epevents, &ev) == -1) {
        return -1;
    }

    if (ev.type == EV_SYN) {
        return 0;
    }

    if (ev.type == EV_KEY && ev.code <= KEY_MAX) {
        int code = getMapKey(ev.code);
        if (code > 0) {
            ProcessKey(code, ev.value);
        } else {
            ProcessKey(ev.code, ev.value);
        }
    }

    return 0;
}

// Process a key-up or -down event.  A key is "registered" when it is
// pressed and then released, with no other keypresses or releases in
// between.  Registered keys are passed to CheckKey() to see if it
// should trigger a visibility toggle, an immediate reboot, or be
// queued to be processed next time the foreground thread wants a key
// (eg, for the menu).
//
// We also keep track of which keys are currently down so that
// CheckKey can call IsKeyPressed to see what other keys are held when
// a key is registered.
//
// value == 1 for key down events; 0 for key up events;2 for key repeat events
void EventsProcess::ProcessKey(int key_code, int value) {
    bool register_key = false;
    bool long_press = false;

    pthread_mutex_lock(&key_queue_mutex);
    key_pressed[key_code] = value;
    if (value == 1) {
        ++key_down_count;
        key_last_down = key_code;
        key_long_press = false;
        key_timer_t* info = new key_timer_t;
        info->ep = this;
        info->key_code = key_code;
        info->count = key_down_count;
        pthread_t thread;
        pthread_create(&thread, nullptr, &EventsProcess::time_key_helper, info);
        pthread_detach(thread);
    } else if(value == 2){
        long_press = key_long_press = true;
	printf("%s,get kernel report repeat event\n",__func__);
    } else {
        if (key_last_down == key_code) {
            long_press = key_long_press;
            register_key = true;
        }
        key_last_down = -1;
    }
    pthread_mutex_unlock(&key_queue_mutex);

    if (register_key) {
        switch (CheckKey(key_code, long_press)) {
          case EventsProcess::IGNORE:
            break;

          case EventsProcess::LONGPRESS:
            break;

	  case EventsProcess::TOGGLE:
            break;

	  case EventsProcess::REBOOT:
            break;

          case EventsProcess::ENQUEUE:
            EnqueueKey(key_code);
            break;
        }
    }
}

void* EventsProcess::time_key_helper(void* cookie) {
    key_timer_t* info = (key_timer_t*) cookie;
    info->ep->time_key(info->key_code, info->count);
    delete info;
    return nullptr;
}

void EventsProcess::time_key(int key_code, int count) {
    usleep(750000);  // 750 ms == "long"
    bool long_press = false;
    pthread_mutex_lock(&key_queue_mutex);
    if (key_last_down == key_code && key_down_count == count) {
        long_press = key_long_press = true;
    }
    pthread_mutex_unlock(&key_queue_mutex);
    if (long_press)
	KeyLongPress(key_code);
}

int EventsProcess::getKey(char *key) {
    if (key == NULL) return -1;

    unsigned int i;
    for (i = 0; i < NUM_CTRLINFO; i++) {
        CtrlInfo_t *info = &g_ctrlinfo[i];
        if (strcmp(info->type, key) == 0) {
            return info->value;
        }
    }
    return -1;
}

void EventsProcess::load_key_map() {
    FILE* fstab = fopen("/etc/gpio_key.kl", "r");
    if (fstab != NULL) {
        printf("loaded /etc/gpio_key.kl\n");
        int alloc = 2;
        keys_map = (KeyMapItem_t*)malloc(alloc * sizeof(KeyMapItem_t));

        keys_map[0].type = "down";
        keys_map[0].value = KEY_DOWN;
        keys_map[0].key[0] = -1;
        keys_map[0].key[1] = -1;
        keys_map[0].key[2] = -1;
        keys_map[0].key[3] = -1;
        keys_map[0].key[4] = -1;
        keys_map[0].key[5] = -1;
        num_keys = 0;

        char buffer[1024];
        int i;
        int value = -1;
        while (fgets(buffer, sizeof(buffer)-1, fstab)) {
            for (i = 0; buffer[i] && isspace(buffer[i]); ++i);

            if (buffer[i] == '\0' || buffer[i] == '#') continue;

            char* original = strdup(buffer);

            char* type = strtok(original+i, " \t\n");
            char* key1 = strtok(NULL, " \t\n");
            char* key2 = strtok(NULL, " \t\n");
            char* key3 = strtok(NULL, " \t\n");
            char* key4 = strtok(NULL, " \t\n");
            char* key5 = strtok(NULL, " \t\n");
            char* key6 = strtok(NULL, " \t\n");

            value = getKey(type);
            if (type && key1 && (value > 0)) {
                while (num_keys >= alloc) {
                    alloc *= 2;
                    keys_map = (KeyMapItem_t*)realloc(keys_map, alloc*sizeof(KeyMapItem_t));
                }
                keys_map[num_keys].type = strdup(type);
                keys_map[num_keys].value = value;
                keys_map[num_keys].key[0] = key1?atoi(key1):-1;
                keys_map[num_keys].key[1] = key2?atoi(key2):-1;
                keys_map[num_keys].key[2] = key3?atoi(key3):-1;
                keys_map[num_keys].key[3] = key4?atoi(key4):-1;
                keys_map[num_keys].key[4] = key5?atoi(key5):-1;
                keys_map[num_keys].key[5] = key6?atoi(key6):-1;

                ++num_keys;
            } else {
                printf("error: skipping malformed keyboard.lk line: %s\n", original);
            }
            free(original);
        }

        fclose(fstab);
    } else {
        printf("error: failed to open /etc/gpio_key.kl, use default map\n");
        num_keys = NUM_DEFAULT_KEY_MAP;
        keys_map = g_default_keymap;
    }

    printf("keyboard key map table:\n");
    int i;
    for (i = 0; i < num_keys; ++i) {
        KeyMapItem_t* v = &keys_map[i];
        printf("  %d type:%s value:%d key:%d %d %d %d %d %d\n", i, v->type, v->value,
              v->key[0], v->key[1], v->key[2], v->key[3], v->key[4], v->key[5]);
    }
    printf("\n");
}

int EventsProcess::getMapKey(int key) {
	int i,j;
	for (i = 0; i < num_keys; i++) {
		KeyMapItem_t* v = &keys_map[i];
		for (j = 0; j < 6; j++) {
			if (v->key[j] == key)
				return v->value;
		}
	}

	return -1;
}

// retrun time interval in millisecond between two timeval.
long get_time_diff(struct timeval before, struct timeval later) {
    long before_sec = before.tv_sec;
    long before_usec = before.tv_usec;
    long later_sec = later.tv_sec;
    long later_usec = later.tv_usec;

    return (later_sec - before_sec) * 1000 + (later_usec - before_usec) / 1000;
}

void EventsProcess::EnqueueKey(int key_code) {
    struct timeval now;
    gettimeofday(&now, nullptr);

    pthread_mutex_lock(&key_queue_mutex);
    const int queue_max = sizeof(key_queue) / sizeof(key_queue[0]);
    if (key_queue_len < queue_max) {
        if (last_key != key_code || get_time_diff(last_queue_time, now) >= KEY_EVENT_TIME_INTERVAL) {
            key_queue[key_queue_len++] = key_code;
            last_queue_time = now;
        }
        pthread_cond_signal(&key_queue_cond);
    }
    pthread_mutex_unlock(&key_queue_mutex);
}

int EventsProcess::WaitKey() {
    pthread_mutex_lock(&key_queue_mutex);

    // Time out after WAIT_KEY_TIMEOUT_SEC, unless a USB cable is
    // plugged in.
    do {
        struct timeval now;
        struct timespec timeout;
        gettimeofday(&now, nullptr);
        timeout.tv_sec = now.tv_sec;
        timeout.tv_nsec = now.tv_usec * 1000;
        timeout.tv_sec += WAIT_KEY_TIMEOUT_SEC;

        int rc = 0;
        while (key_queue_len == 0 && rc != ETIMEDOUT) {
            rc = pthread_cond_timedwait(&key_queue_cond, &key_queue_mutex, &timeout);
        }
    } while (key_queue_len == 0);

    int key = -1;
    if (key_queue_len > 0) {
        key = key_queue[0];
        memcpy(&key_queue[0], &key_queue[1], sizeof(int) * --key_queue_len);
    }
    pthread_mutex_unlock(&key_queue_mutex);
    return key;
}

bool EventsProcess::IsKeyPressed(int key) {
    pthread_mutex_lock(&key_queue_mutex);
    int pressed = key_pressed[key];
    pthread_mutex_unlock(&key_queue_mutex);
    return pressed;
}

bool EventsProcess::IsLongPress() {
    pthread_mutex_lock(&key_queue_mutex);
    bool result = key_long_press;
    pthread_mutex_unlock(&key_queue_mutex);
    return result;
}

bool EventsProcess::HasThreeButtons() {
    return has_power_key && has_up_key && has_down_key;
}

void EventsProcess::FlushKeys() {
    pthread_mutex_lock(&key_queue_mutex);
    key_queue_len = 0;
    pthread_mutex_unlock(&key_queue_mutex);
}

EventsProcess::KeyAction EventsProcess::CheckKey(int key, bool is_long_press) {
    pthread_mutex_lock(&key_queue_mutex);
    key_long_press = false;
    pthread_mutex_unlock(&key_queue_mutex);

    // If we have power and volume up keys, that chord is the signal to toggle the text display.
    if (HasThreeButtons()) {
        if (key == KEY_VOLUMEUP && IsKeyPressed(KEY_POWER)) {
            return TOGGLE;
        }
    }

    if (is_long_press) {
         return LONGPRESS;
    }

    last_key = key;
    return ENQUEUE;
}

void EventsProcess::KeyLongPress(int) {
	printf("bebond timer generate %s\n",__func__);
	key_long_press = false;
}

