# Copyright 2018 The Cobalt Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
{
  'variables': {
# oemctyptoimpl optee ref
    'oemctyptoimpl':'optee',
  },
  'targets': [
    {
      'target_name': 'widevine_cdm_cobalt_none',
      'type': 'none',
      'conditions': [
          ['oemctyptoimpl=="optee"', {
              'all_dependent_settings': {
                  'defines': [
                      'COBALT_WIDEVINE_OPTEE',
                  ],
                  'libraries': [
                      '-loemcrypto',
                      '-lsecmem',
                      '-lteec',
                  ],},
          }],  # oemctyptoimpl=="optee"
      ],  # conditions
    },
    {
      'target_name': 'oemcrypto_ref',
      'type': 'static_library',
      'includes': [
        '<(DEPTH)/third_party/starboard/amlogic/shared/ce_cdm/oemcrypto/ref/oec_ref.gypi',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(DEPTH)/third_party/starboard/amlogic/shared/ce_cdm/core/include',
          '<(DEPTH)/third_party/starboard/amlogic/shared/ce_cdm/oemcrypto/include',
        ],
      },
    },
    {
      'target_name': 'oemcrypto_optee',
      'type': 'none',
      'direct_dependent_settings': {
        'include_dirs': [
          '<(DEPTH)/third_party/starboard/amlogic/shared/ce_cdm/core/include',
          '<(DEPTH)/third_party/starboard/amlogic/shared/ce_cdm/oemcrypto/include',
        ],
      },
    },
    {
      'target_name': 'widevine_cdm_cobalt',
      'type': 'shared_library',
      'defines': [
        'STARBOARD_IMPLEMENTATION',
      ],
      'sources': [
          '<(DEPTH)/third_party/starboard/amlogic/shared/widevine/drm_create_system_dl.cc',
#          '<(DEPTH)/third_party/starboard/amlogic/shared/widevine/drm_create_system.cc',
#          '<(DEPTH)/third_party/starboard/amlogic/shared/widevine/drm_system_widevine.cc',
#          '<(DEPTH)/third_party/starboard/amlogic/shared/widevine/drm_system_widevine.h',
#          '<(DEPTH)/third_party/starboard/amlogic/shared/widevine/widevine_storage.cc',
#          '<(DEPTH)/third_party/starboard/amlogic/shared/widevine/widevine_storage.h',
#          '<(DEPTH)/third_party/starboard/amlogic/shared/widevine/widevine_timer.cc',
#          '<(DEPTH)/third_party/starboard/amlogic/shared/widevine/widevine_timer.h',
      ],
      'dependencies': [
        '<(boringssl_dependency)',
        '<(DEPTH)/third_party/starboard/amlogic/shared/ce_cdm/cdm/cdm.gyp:widevine_ce_cdm_static',
      ],
      'conditions': [
          ['oemctyptoimpl=="ref"', {
              'dependencies': [
                  'oemcrypto_ref',
              ],  # dependencies
          }, {
              'dependencies': [
                  'oemcrypto_optee',
              ],  # dependencies
              'libraries': [
                '-loemcrypto',
                '-lsecmem',
                '-lteec',
              ]
          }],  # oemctyptoimpl=="ref"
      ],  # conditions
      'libraries!': [
        '-lEGL',
        '-lGLESv2',
        '-lwayland-egl',
        '-lwayland-client',
        '-lasound',
        '-ldl',
        '-lpthread',
        '-lrt',
        '-lamcodec',
        '-lamavutils',
        '-lamadec',
      ],
      'cflags': [
        '-fPIC',
        '-fvisibility=hidden',
      ],
      'ldflags': [
        '-Wl,--exclude-libs,ALL',
        '-fvisibility=hidden',
      ],
    },
  ],
}
