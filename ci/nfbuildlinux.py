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


class NFBuildLinux(NFBuild):
    clang_format_binary = 'clang-format-4.0'
    def __init__(self):
        super(self.__class__, self).__init__()
        self.project_file = 'build.ninja'

    def generateProject(self,
                        code_coverage=False,
                        address_sanitizer=False,
                        thread_sanitizer=False,
                        undefined_behaviour_sanitizer=False,
                        ios=False,
                        gcc=False):
        cmake_call = [
            'cmake',
            '..',
            '-GNinja']
        if gcc:
            cmake_call.extend(['-DLLVM_STDLIB=0'])
        else:
            cmake_call.extend(['-DLLVM_STDLIB=1'])
        cmake_result = subprocess.call(cmake_call, cwd=self.build_directory)
        if cmake_result != 0:
            sys.exit(cmake_result)

    def targetBinary(self, target):
        for root, dirnames, filenames in os.walk(self.build_directory):
            for filename in fnmatch.filter(filenames, target):
                full_target_file = os.path.join(root, filename)
                return full_target_file
        return ''

    def buildTarget(self, target, sdk='linux', arch='x86_64'):
        result = subprocess.call([
            'ninja',
            '-C',
            self.build_directory,
            '-f',
            self.project_file,
            target])
        if result != 0:
            sys.exit(result)

    def packageArtifacts(self):
        lib_name = 'libNFParam.a'
        output_folder = os.path.join(self.build_directory, 'output')
        artifacts_folder = os.path.join(output_folder, 'NFParam')
        shutil.copytree('include', os.path.join(artifacts_folder, 'include'))
        source_folder = os.path.join(self.build_directory, 'source')
        lib_matches = self.find_file(source_folder, lib_name)
        shutil.copyfile(lib_matches[0], os.path.join(artifacts_folder, lib_name))
        output_zip = os.path.join(output_folder, 'libNFParam.zip')
        self.make_archive(artifacts_folder, output_zip)
