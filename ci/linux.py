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

import os
import sys

from nfbuildlinux import NFBuildLinux
from build_options import BuildOptions


def main():
    buildOptions = BuildOptions()
    buildOptions.addOption("debug", "Enable Debug Mode")
    buildOptions.addOption("installDependencies", "Install dependencies")
    buildOptions.addOption("lintCmake", "Lint cmake files")
    buildOptions.addOption("lintCpp", "Lint CPP Files")
    buildOptions.addOption("lintCppWithInlineChange",
                           "Lint CPP Files and fix them")
    buildOptions.addOption("unitTests", "Run Unit Tests")
    buildOptions.addOption("makeBuildDirectory",
                           "Wipe existing build directory")
    buildOptions.addOption("generateProject", "Regenerate xcode project")
    buildOptions.addOption("buildTargetLibrary", "Build Target: Library")
    buildOptions.addOption("gnuToolchain", "Build with gcc and libstdc++")
    buildOptions.addOption("llvmToolchain", "Build with clang and libc++")

    buildOptions.setDefaultWorkflow("Empty workflow", [])

    buildOptions.addWorkflow("lint", "Run lint workflow", [
        'installDependencies',
        'lintCmake',
        'lintCppWithInlineChange'
    ])

    buildOptions.addWorkflow("clang_build", "Production Build", [
        'llvmToolchain',
        'installDependencies',
        'lintCmake',
        'lintCpp',
        'makeBuildDirectory',
        'generateProject',
        'buildTargetLibrary',
        'unitTests'
    ])

    buildOptions.addWorkflow("gcc_build", "Production Build", [
        'gnuToolchain',
        'installDependencies',
        'lintCmake',
        'lintCpp',
        'makeBuildDirectory',
        'generateProject',
        'buildTargetLibrary',
        'unitTests'
    ])

    options = buildOptions.parseArgs()
    buildOptions.verbosePrintBuildOptions(options)

    library_target = 'NFParamTests'
    nfbuild = NFBuildLinux()

    if buildOptions.checkOption(options, 'debug'):
        nfbuild.build_type = 'Debug'

    if buildOptions.checkOption(options, 'installDependencies'):
        nfbuild.installDependencies()

    if buildOptions.checkOption(options, 'lintCmake'):
        nfbuild.lintCmake()

    if buildOptions.checkOption(options, 'lintCppWithInlineChange'):
        nfbuild.lintCPP(make_inline_changes=True)
    elif buildOptions.checkOption(options, 'lintCpp'):
        nfbuild.lintCPP(make_inline_changes=False)

    if buildOptions.checkOption(options, 'makeBuildDirectory'):
        nfbuild.makeBuildDirectory()

    if buildOptions.checkOption(options, 'generateProject'):
        if buildOptions.checkOption(options, 'gnuToolchain'):
            os.environ['CC'] = 'gcc-4.9'
            os.environ['CXX'] = 'g++-4.9'
            nfbuild.generateProject(gcc=True)
        elif buildOptions.checkOption(options, 'llvmToolchain'):
            os.environ['CC'] = 'clang-3.9'
            os.environ['CXX'] = 'clang++-3.9'
            nfbuild.generateProject(gcc=False)
        else:
            nfbuild.generateProject()

    if buildOptions.checkOption(options, 'buildTargetLibrary'):
        nfbuild.buildTarget(library_target)

    if buildOptions.checkOption(options, 'unitTests'):
        nfbuild.runUnitTests()

if __name__ == "__main__":
    main()
