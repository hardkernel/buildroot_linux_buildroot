/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 *     ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef APP_H
#define APP_H

bool load_app();
void init_app(int width, int height);
void update_app(float delta_time, float total_time);
void render_app(float delta_time, float total_time);
void free_app();
void on_pointer_down(float x, float y);
void on_pointer_up(float x, float y);

#endif
