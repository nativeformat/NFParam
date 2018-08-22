#!/usr/bin/env python

import sys

from nfbuildosx import NFBuildOSX
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
    buildOptions.addOption("addressSanitizer",
                           "Enable Address Sanitizer in generate project")
    buildOptions.addOption("codeCoverage",
                           "Enable code coverage in generate project")

    buildOptions.addOption("buildTargetLibrary", "Build Target: Library")

    buildOptions.setDefaultWorkflow("Empty workflow", [])

    buildOptions.addWorkflow("local_unit", "Run local unit tests", [
        'debug',
        'installDependencies',
        'lintCmake',
        'unitTests'
    ])

    buildOptions.addWorkflow("lint", "Run lint workflow", [
        'debug',
        'installDependencies',
        'lintCmake',
        'lintCppWithInlineChange'
    ])

    buildOptions.addWorkflow("build", "Production Build", [
        'debug',
        'installDependencies',
        'lintCmake',
        'lintCpp',
        'makeBuildDirectory',
        'generateProject',
        'buildTargetLibrary',
        'unitTests'
    ])

    buildOptions.addWorkflow("code_coverage", "Collect code coverage", [
        'debug',
        'installDependencies',
        'lintCmake',
        'lintCpp',
        'makeBuildDirectory',
        'generateProject',
        'codeCoverage',
        'unitTests'
    ])

    options = buildOptions.parseArgs()
    buildOptions.verbosePrintBuildOptions(options)

    library_target = 'NFParamTests'
    nfbuild = NFBuildOSX()

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
        nfbuild.generateProject(
            code_coverage='codeCoverage' in options,
            address_sanitizer='addressSanitizer' in options
            )

    if buildOptions.checkOption(options, 'buildTargetLibrary'):
        nfbuild.buildTarget(library_target)

    if buildOptions.checkOption(options, 'unitTests'):
        nfbuild.runUnitTests()

    if buildOptions.checkOption(options, 'codeCoverage'):
        nfbuild.collectCodeCoverage()

if __name__ == "__main__":
    main()
