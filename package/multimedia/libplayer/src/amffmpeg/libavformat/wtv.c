/*
 * Windows Television (WTV)
 * Copyright (c) 2010-2011 Peter Ross <pross@xvid.org>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "wtv.h"

/* WTV GUIDs*/
const ff_asf_guid ff_dir_entry_guid =
    {0x92,0xB7,0x74,0x91,0x59,0x70,0x70,0x44,0x88,0xDF,0x06,0x3B,0x82,0xCC,0x21,0x3D};
const ff_asf_guid ff_wtv_guid =
    {0xB7,0xD8,0x00,0x20,0x37,0x49,0xDA,0x11,0xA6,0x4E,0x00,0x07,0xE9,0x5E,0xAD,0x8D};
const ff_asf_guid ff_timestamp_guid =
    {0x5B,0x05,0xE6,0x1B,0x97,0xA9,0x49,0x43,0x88,0x17,0x1A,0x65,0x5A,0x29,0x8A,0x97};
const ff_asf_guid ff_data_guid =
    {0x95,0xC3,0xD2,0xC2,0x7E,0x9A,0xDA,0x11,0x8B,0xF7,0x00,0x07,0xE9,0x5E,0xAD,0x8D};
const ff_asf_guid ff_stream_guid =
    {0xED,0xA4,0x13,0x23,0x2D,0xBF,0x4F,0x45,0xAD,0x8A,0xD9,0x5B,0xA7,0xF9,0x1F,0xEE};
const ff_asf_guid ff_mediatype_audio =
    {'a','u','d','s',FF_MEDIASUBTYPE_BASE_GUID};
const ff_asf_guid ff_mediatype_video =
    {'v','i','d','s',FF_MEDIASUBTYPE_BASE_GUID};
const ff_asf_guid ff_format_none =
    {0xD6,0x17,0x64,0x0F,0x18,0xC3,0xD0,0x11,0xA4,0x3F,0x00,0xA0,0xC9,0x22,0x31,0x96};

const AVCodecGuid ff_video_guids[] = {
    {CODEC_ID_MPEG2VIDEO, {0x26,0x80,0x6D,0xE0,0x46,0xDB,0xCF,0x11,0xB4,0xD1,0x00,0x80,0x5F,0x6C,0xBB,0xEA}},
    {CODEC_ID_NONE}
};
