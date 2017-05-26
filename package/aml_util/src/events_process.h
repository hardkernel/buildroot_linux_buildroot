/*
 * Copyright (C) Amlogic
 *
 * author: peipeng.zhao@amlogic.com
 */


#ifndef EVENTS_PROCESS_H
#define EVENTS_PROCESS_H

#include <linux/input.h>
#include <pthread.h>
#include <time.h>
#include <stdint.h>
#include <sys/time.h>
// Abstract class for controlling the user interface during recovery.
class EventsProcess {
  public:
    struct KeyMapItem_t {
        const char* type;
        int value;
        int key[6];
    };

    struct CtrlInfo_t {
        const char *type;
        int value;
    };

    EventsProcess();

    virtual ~EventsProcess() { }

    // Initialize the object; called before anything else.
    virtual void Init();
    // --- key handling ---

    // Wait for a key and return it.  May return -1 after timeout.
    virtual int WaitKey();

    virtual bool IsKeyPressed(int key);
    virtual bool IsLongPress();

    // Returns true if you have the volume up/down and power trio typical
    // of phones and tablets, false otherwise.
    virtual bool HasThreeButtons();

    // Erase any queued-up keys.
    virtual void FlushKeys();

    // Called on each key press, even while operations are in progress.
    // Return value indicates whether an immediate operation should be
    // triggered (toggling the display, rebooting the device), or if
    // the key should be enqueued for use by the main thread.
    enum KeyAction { ENQUEUE, TOGGLE, REBOOT, IGNORE , LONGPRESS};
    virtual KeyAction CheckKey(int key, bool is_long_press);

    // Called when a key is held down long enough to have been a
    // long-press (but before the key is released).  This means that
    // if the key is eventually registered (released without any other
    // keys being pressed in the meantime), CheckKey will be called with
    // 'is_long_press' true.
    virtual void KeyLongPress(int key);


protected:
    void EnqueueKey(int key_code);

private:
    // Key event input queue
    pthread_mutex_t key_queue_mutex;
    pthread_cond_t key_queue_cond;
    int key_queue[256], key_queue_len;
    struct timeval last_queue_time;
    char key_pressed[KEY_MAX + 1];     // under key_queue_mutex
    int key_last_down;                 // under key_queue_mutex
    bool key_long_press;               // under key_queue_mutex
    int key_down_count;                // under key_queue_mutex
    bool enable_reboot;                // under key_queue_mutex
    int rel_sum;

    int consecutive_power_keys;
    int last_key;

    bool has_power_key;
    bool has_up_key;
    bool has_down_key;

    struct key_timer_t {
        EventsProcess* ep;
        int key_code;
        int count;
    };

    int num_keys;
    KeyMapItem_t* keys_map;


    #define NUM_DEFAULT_KEY_MAP 3


    #define NUM_CTRLINFO 3

    pthread_t input_thread_;

    void OnKeyDetected(int key_code);

    static int InputCallback(int fd, uint32_t epevents, void* data);
    int OnInputEvent(int fd, uint32_t epevents);
    void ProcessKey(int key_code, int updown);


    static void* time_key_helper(void* cookie);
    void time_key(int key_code, int count);
    int getKey(char *key);
    void load_key_map();
    int getMapKey(int key);
};

#endif  // RECOVERY_UI_H
