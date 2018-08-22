#!/usr/bin/env python

import fnmatch
import os
import pprint
import shutil
import subprocess
import sys
import yaml


class NFBuild(object):
    def __init__(self):
        ci_yaml_file = os.path.join('ci', 'ci.yaml')
        self.build_configuration = yaml.load(open(ci_yaml_file, 'r'))
        self.build_type = 'Release'
        self.pretty_printer = pprint.PrettyPrinter(indent=4)
        self.current_working_directory = os.getcwd()
        self.build_directory = 'build'
        self.output_directory = os.path.join(self.build_directory, 'output')
        self.statically_analyzed_files = []

    def build_print(self, print_string):
        print print_string
        sys.stdout.flush()

    def makeBuildDirectory(self):
        if os.path.exists(self.build_directory):
            shutil.rmtree(self.build_directory)
        os.makedirs(self.build_directory)
        os.makedirs(self.output_directory)

    def installDependencies(self):
        pass

    def generateProject(self,
                        code_coverage=False,
                        electron_build=False,
                        address_sanitizer=False,
                        thread_sanitizer=False,
                        undefined_behaviour_sanitizer=False,
                        ios=False):
        assert True, "generateProject should be overridden by subclass"

    def buildTarget(self, target, sdk='macosx'):
        assert True, "buildTarget should be overridden by subclass"

    def lintCPPFile(self, filepath, make_inline_changes=False):
        current_source = open(filepath, 'r').read()
        clang_format_call = [self.clang_format_binary, '-style=file']
        if make_inline_changes:
            clang_format_call.append('-i')
        clang_format_call.append(filepath)
        new_source = subprocess.check_output(clang_format_call)
        if current_source != new_source and not make_inline_changes:
            self.build_print(
                filepath + " failed C++ lint, file should look like:")
            self.build_print(new_source)
            return False
        return True

    def lintCPPDirectory(self, directory, make_inline_changes=False):
        passed = True
        for root, dirnames, filenames in os.walk(directory):
            for filename in filenames:
                if not filename.endswith(('.cpp', '.h', '.m', '.mm')):
                    continue
                full_filepath = os.path.join(root, filename)
                if not self.lintCPPFile(full_filepath, make_inline_changes):
                    passed = False
        return passed

    def lintCPP(self, make_inline_changes=False):
        lint_result = self.lintCPPDirectory('source', make_inline_changes)
        lint_result &= self.lintCPPDirectory('include', make_inline_changes)
        if not lint_result:
            sys.exit(1)

    def lintCmakeFile(self, filepath):
        self.build_print("Linting: " + filepath)
        return subprocess.call(['cmakelint', filepath]) == 0

    def lintCmakeDirectory(self, directory):
        passed = True
        for root, dirnames, filenames in os.walk(directory):
            for filename in filenames:
                if not filename.endswith('CMakeLists.txt'):
                    continue
                full_filepath = os.path.join(root, filename)
                if not self.lintCmakeFile(full_filepath):
                    passed = False
        return passed

    def targetBinary(self, target):
        assert True, "targetBinary should be overridden by subclass"

    def lintCmake(self):
        lint_result = self.lintCmakeFile('CMakeLists.txt')
        lint_result &= self.lintCmakeDirectory('source')
        if not lint_result:
            sys.exit(1)

    def runTarget(self, target):
        target_file = self.targetBinary(target)
        target_result = subprocess.call([target_file])
        if target_result:
            sys.exit(target_result)

    def runUnitTests(self):
        for unit_test_target in self.build_configuration['unit_tests']:
            self.buildTarget(unit_test_target)
            self.runTarget(unit_test_target)

    def collectCodeCoverage(self):
        for root, dirnames, filenames in os.walk('build'):
            for filename in fnmatch.filter(filenames, '*.gcda'):
                full_filepath = os.path.join(root, filename)
                if full_filepath.startswith('build/source/') and \
                    '/test/' not in full_filepath and \
                    '/usr/include/c++/' not in full_filepath:
                    continue
                os.remove(full_filepath)
        llvm_run_script = os.path.join(
            os.path.join(
                self.current_working_directory,
                'ci'),
            'llvm-run.sh')
        cov_info = os.path.join('build', 'cov.info')
        filtered_cov_info = os.path.join('build', 'filtered_cov.info')
        
        # Run lcov on filtered info
        lcov_result = subprocess.call([
            'lcov',
            '--directory',
            '.',
            '--base-directory',
            '.',
            '--gcov-tool',
            llvm_run_script,
            '--capture',
            '-o',
            cov_info])
        if lcov_result:
            sys.exit(lcov_result)

        # Remove stdlib headers from files to check
        lcov_result = subprocess.call([
            'lcov',
            '--remove',
            cov_info,
            '*/usr/include/c++/*',
            '-o',
            filtered_cov_info])
        if lcov_result:
            sys.exit(lcov_result)

        coverage_output = os.path.join(self.output_directory, 'code_coverage')
        genhtml_result = subprocess.call([
            self.genhtml_binary,
            filtered_cov_info,
            '-o',
            coverage_output])
        if genhtml_result:
            sys.exit(genhtml_result)
