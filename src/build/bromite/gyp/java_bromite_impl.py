#!/usr/bin/env python3

import argparse
import os
import re
import sys
import zipfile

sys.path.append('build/android/gyp')
sys.path.append('../../build/android/gyp')

from util import build_utils
from util import java_cpp_utils
import action_helpers  # build_utils adds //build to sys.path.
import zip_helpers

def _GenerateOutput(template, source_paths, template_path, strings,
                    namespace, static_classes):
  description_template = """
// This following string constants were inserted by
//     {SCRIPT_NAME}
// Directory
//     {SOURCE_PATHS}
// Template
//     {TEMPLATE_PATH}

"""
  values = {
      'SCRIPT_NAME': java_cpp_utils.GetScriptName(),
      'SOURCE_PATHS': ',\n//    '.join(source_paths),
      'TEMPLATE_PATH': template_path,
  }
  description = description_template.format(**values)

  import_clause = '\n'.join(
      ['import ' + namespace + '.' + f + ';' for f in strings])

  if static_classes:
    add_clause = '\n\t'.join(['\tadd(' + f + '.getInstance());' for f in strings])
  else:
    add_clause = '\n\t'.join(['\tadd(new ' + f + '());' for f in strings])

  values = {
      'DESCRIPTION': description,
      'ADD_CLAUSE': add_clause,
      'IMPORT_CLAUSE': import_clause,
  }
  return template.format(**values)


def _Generate(source_paths, template_path, namespace, static_classes):
  with open(template_path) as f:
    lines = f.readlines()

  template = ''.join(lines)
  package, class_name = java_cpp_utils.ParseTemplateFile(lines)
  output_path = java_cpp_utils.GetJavaFilePath(package, class_name)
  strings = []

  for directory in source_paths:
    if not directory.endswith('include_all_directory.java'):
      raise Exception("input path not ends with 'include_all_directory.java'")
    for root, dirs, directory_files in os.walk(os.path.dirname(directory)):
      for directory_file in sorted(directory_files):
        strings.append(directory_file)

  strings = [f.replace('.java', '') for f in strings
             if not f.endswith('include_all_directory.java') and
                not f.endswith('.java.tmpl') ]

  output = _GenerateOutput(template, source_paths, template_path, strings,
                           namespace, static_classes)

  return output, output_path


def _Main(argv):
  parser = argparse.ArgumentParser()

  parser.add_argument('--srcjar',
                      required=True,
                      help='The path at which to generate the .srcjar file')

  parser.add_argument('--template',
                      required=True,
                      help='The template file with which to generate the Java '
                      'class.')

  parser.add_argument(
      '--namespace', required=True, help='Output namespace')
  parser.add_argument(
      '--static_classes', action="store_true",
      default=False,
      help='True to use getInstance() method')
  parser.add_argument(
      'inputs', nargs='+', help='Input folder(s)', metavar='INPUTFILE')
  args = parser.parse_args(argv)

  with action_helpers.atomic_output(args.srcjar) as f:
    with zipfile.ZipFile(f, 'w', zipfile.ZIP_STORED) as srcjar:
      data, path = _Generate(args.inputs, args.template,
                             args.namespace, args.static_classes)
      zip_helpers.add_to_zip_hermetic(srcjar, path, data=data)


if __name__ == '__main__':
  _Main(sys.argv[1:])