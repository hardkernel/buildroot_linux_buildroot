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
"""Thirdparty Starboard Amlogic arm Wayland platform configuration."""
import os
import sys

from third_party.starboard.amlogic.shared import gyp_configuration as shared_configuration
from starboard.build import clang
from starboard.tools import build

class ArmWaylandConfiguration(shared_configuration.LinuxConfiguration):
  """Thirdparty Starboard Amlogic arm Wayland platform configuration."""

  def __init__(self,
               platform_name='amlogic-wayland-armv7l',
               asan_enabled_by_default=False,
               goma_supports_compiler=True):
    super(ArmWaylandConfiguration, self).__init__(
        platform_name, asan_enabled_by_default, goma_supports_compiler)

  def GetEnvironmentVariables(self):
    if not hasattr(self, 'host_compiler_environment'):
      self.host_compiler_environment = build.GetHostCompilerEnvironment(
          clang.GetClangSpecification(), self.goma_supports_compiler)

    env_variables = self.host_compiler_environment
    env_variables.update({
        'CC': "arm-linux-gnueabihf-gcc",
        'CXX': "arm-linux-gnueabihf-g++",
    })
    return env_variables

  def GetVariables(self, config_name):
    variables = super(ArmWaylandConfiguration, self).GetVariables(
        config_name)
    sysroot = os.environ['SYS_ROOT'];
    variables.update({
        'sysroot': sysroot,
    })
    return variables

def CreatePlatformConfig():
  return ArmWaylandConfiguration()
