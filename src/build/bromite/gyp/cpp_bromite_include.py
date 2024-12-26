#!/usr/bin/env python3

import argparse
import os
import re
import sys

def _Main(argv):
  parser = argparse.ArgumentParser()

  parser.add_argument('--output',
                      required=True,
                      help='The path at which to generate the .inc file')

  parser.add_argument(
      'inputs', nargs='+', help='Input folder(s)', metavar='INPUTFILE')
  args = parser.parse_args(argv)

  for include_file in args.inputs:
    directory = os.path.dirname(include_file)
    files = []
    for root, dirs, directory_files in os.walk(directory):
      for directory_file in sorted(directory_files):
        files.append(os.path.join(directory, directory_file))

    files = [f for f in files if f.endswith('.inc')]

  with open(args.output, 'w') as f:
    for file in files:
      f.write("#include \"" + file + "\"\n")
    f.write("\n")


if __name__ == '__main__':
  _Main(sys.argv[1:])