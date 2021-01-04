#!/usr/bin/env python3
#
#===- run-clang-tidy.py - Parallel clang-tidy runner --------*- python -*--===#
#
# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
#===-----------------------------------------------------------------------===#
# FIXME: Integrate with clang-tidy-diff.py


"""
Parallel clang-tidy runner
==========================

Runs clang-tidy over all files in a compilation database. Requires clang-tidy
and clang-apply-replacements in $PATH.

Example invocations.
- Run clang-tidy on all files in the current working directory with a default
  set of checks and show warnings in the cpp files and all project headers.
    run-clang-tidy.py $PWD

- Fix all header guards.
    run-clang-tidy.py -fix -checks=-*,llvm-header-guard

- Fix all header guards included from clang-tidy and header guards
  for clang-tidy headers.
    run-clang-tidy.py -fix -checks=-*,llvm-header-guard extra/clang-tidy \
                      -header-filter=extra/clang-tidy

Compilation database setup:
http://clang.llvm.org/docs/HowToSetupToolingForLLVM.html
"""

from __future__ import print_function

import argparse
import glob
import json
import multiprocessing
import os
import re
import shutil
import subprocess
import sys
import tempfile
import threading
import traceback

try:
  import yaml
except ImportError:
  yaml = None

is_py2 = sys.version[0] == '2'

if is_py2:
    import Queue as queue
else:
    import queue as queue


def make_absolute(f, directory):
  if os.path.isabs(f):
    return f
  return os.path.normpath(os.path.join(directory, f))


def get_tidy_invocation(f, clang_tidy_binary, header_filter):
  """Gets a command line for clang-tidy."""
  start = [clang_tidy_binary, f]
  if header_filter is not None:
    start.append('-header-filter=' + header_filter)
  start.append('--')
  start.append('-Icugl/include')
  return start


def merge_replacement_files(tmpdir, mergefile):
  """Merge all replacement files in a directory into a single file"""
  # The fixes suggested by clang-tidy >= 4.0.0 are given under
  # the top level key 'Diagnostics' in the output yaml files
  mergekey = "Diagnostics"
  merged=[]
  for replacefile in glob.iglob(os.path.join(tmpdir, '*.yaml')):
    content = yaml.safe_load(open(replacefile, 'r'))
    if not content:
      continue # Skip empty files.
    merged.extend(content.get(mergekey, []))

  if merged:
    # MainSourceFile: The key is required by the definition inside
    # include/clang/Tooling/ReplacementsYaml.h, but the value
    # is actually never used inside clang-apply-replacements,
    # so we set it to '' here.
    output = {'MainSourceFile': '', mergekey: merged}
    with open(mergefile, 'w') as out:
      yaml.safe_dump(output, out)
  else:
    # Empty the file:
    open(mergefile, 'w').close()


def run_tidy(args, tmpdir, queue, lock, failed_files):
  """Takes filenames out of queue and runs clang-tidy on them."""
  while True:
    name = queue.get()
    invocation = get_tidy_invocation(name, args.clang_tidy_binary, args.header_filter)

    proc = subprocess.Popen(invocation, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    output, err = proc.communicate()
    if proc.returncode != 0:
      failed_files.append(name)
    with lock:
      sys.stdout.write(' '.join(invocation) + '\n' + output.decode('utf-8'))
      if len(err) > 0:
        sys.stdout.flush()
        sys.stderr.write(err.decode('utf-8'))
    queue.task_done()


def main():
  parser = argparse.ArgumentParser(description='Runs clang-tidy over all files '
                                   'in a compilation database. Requires '
                                   'clang-tidy and clang-apply-replacements in '
                                   '$PATH.')
  parser.add_argument('-clang-tidy-binary', metavar='PATH',
                      default='clang-tidy',
                      help='path to clang-tidy binary')
  parser.add_argument('-header-filter', default=None,
                      help='regular expression matching the names of the '
                      'headers to output diagnostics from. Diagnostics from '
                      'the main file of each translation unit are always '
                      'displayed.')
  parser.add_argument('files', nargs='*', default=['.*'],
                      help='files to be processed (regex on path)')
  parser.add_argument('-cpp', nargs='*', help='cpp files to process')
  args = parser.parse_args()

  db_path = 'compile_commands.json'

  files = args.cpp

  max_task = multiprocessing.cpu_count()
  tmpdir = None

  # Build up a big regexy filter from all command line arguments.
  file_name_re = re.compile('|'.join(args.files))

  return_code = 0
  try:
    # Spin up a bunch of tidy-launching threads.
    task_queue = queue.Queue(max_task)
    # List of files with a non-zero return code.
    failed_files = []
    lock = threading.Lock()
    for _ in range(max_task):
      t = threading.Thread(target=run_tidy,
                           args=(args, tmpdir, task_queue, lock, failed_files))
      t.daemon = True
      t.start()

    # Fill the queue with files.
    for name in files:
      if file_name_re.search(name):
        task_queue.put(name)

    # Wait for all threads to be done.
    task_queue.join()
    if len(failed_files):
      return_code = 1

  except KeyboardInterrupt:
    # This is a sad hack. Unfortunately subprocess goes
    # bonkers with ctrl-c and we start forking merrily.
    print('\nCtrl-C detected, goodbye.')
    if tmpdir:
      shutil.rmtree(tmpdir)
    os.kill(0, 9)


  if tmpdir:
    shutil.rmtree(tmpdir)
  sys.exit(return_code)


if __name__ == '__main__':
  main()
