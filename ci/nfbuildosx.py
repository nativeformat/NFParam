#!/usr/bin/env python
'''
 * Copyright (c) 2018 Spotify AB.
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
'''

import fnmatch
import os
import plistlib
import re
import shutil
import subprocess
import sys

from distutils import dir_util
from nfbuild import NFBuild


class NFBuildOSX(NFBuild):
    clang_format_binary = 'clang-format'
    def __init__(self):
        super(self.__class__, self).__init__()
        self.project_file = os.path.join(
            self.build_directory,
            'NFParam.xcodeproj')

    def generateProject(self,
                        code_coverage=False,
                        electron_build=False,
                        address_sanitizer=False,
                        thread_sanitizer=False,
                        undefined_behaviour_sanitizer=False,
                        ios=False):
        cmake_call = [
            'cmake',
            '..',
            '-GXcode']
        if code_coverage:
            cmake_call.append('-DCODE_COVERAGE=1')
        else:
            cmake_call.append('-DCODE_COVERAGE=0')
        if address_sanitizer:
            cmake_call.append('-DUSE_ADDRESS_SANITIZER=1')
        else:
            cmake_call.append('-DUSE_ADDRESS_SANITIZER=0')
        cmake_result = subprocess.call(cmake_call, cwd=self.build_directory)
        if cmake_result != 0:
            sys.exit(cmake_result)

    def buildTarget(self, target, sdk='macosx', arch='x86_64'):
        xcodebuild_result = subprocess.call([
            'xcodebuild',
            '-project',
            self.project_file,
            '-target',
            target,
            '-sdk',
            sdk,
            '-arch',
            arch,
            '-configuration',
            self.build_type,
            'build'])
        if xcodebuild_result != 0:
            sys.exit(xcodebuild_result)

    def targetBinary(self, target):
        for root, dirnames, filenames in os.walk(self.build_directory):
            for filename in fnmatch.filter(filenames, target):
                full_target_file = os.path.join(root, filename)
                return full_target_file
        return ''

    def packageArtifacts(self):
        lib_name = 'libNFParam.a'
        output_folder = os.path.join(self.build_directory, 'output')
        artifacts_folder = os.path.join(output_folder, 'NFParam')
        shutil.copytree('include', os.path.join(artifacts_folder, 'include'))
        source_folder = os.path.join(self.build_directory, 'source')
        lib_matches = self.find_file(source_folder, lib_name)
        lipo_command = ['lipo', '-create']
        for lib_match in lib_matches:
            lipo_command.append(lib_match)
        lipo_command.extend(['-output', os.path.join(artifacts_folder, lib_name)])
        lipo_result = subprocess.call(lipo_command)
        if lipo_result != 0:
            sys.exit(lipo_result)
        output_zip = os.path.join(output_folder, 'libNFParam.zip')
        self.make_archive(artifacts_folder, output_zip)
