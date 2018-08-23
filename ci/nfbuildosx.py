#!/usr/bin/env python

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
