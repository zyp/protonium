from setuptools import setup
from distutils.command.build_py import build_py as _build_py
from distutils.spawn import find_executable
import os
import sys
import subprocess

from protobuf_setuptools import ProtoBuild

## Find the Protocol Compiler.
#if 'PROTOC' in os.environ and os.path.exists(os.environ['PROTOC']):
#  protoc = os.environ['PROTOC']
#elif os.path.exists('../src/protoc'):
#  protoc = '../src/protoc'
#elif os.path.exists('../src/protoc.exe'):
#  protoc = '../src/protoc.exe'
#elif os.path.exists('../vsprojects/Debug/protoc.exe'):
#  protoc = '../vsprojects/Debug/protoc.exe'
#elif os.path.exists('../vsprojects/Release/protoc.exe'):
#  protoc = '../vsprojects/Release/protoc.exe'
#else:
#  protoc = find_executable('protoc')
#
#def GenProto(source, require=True):
#  """Generates a _pb2.py from the given .proto file.
#  Does nothing if the output already exists and is newer than the input.
#  Args:
#      source: the .proto file path.
#      require: if True, exit immediately when a path is not found.
#  """
#
#  if not require and not os.path.exists(source):
#    return
#
#  output = source.replace('.proto', '_pb2.py').replace('../src/', '')
#
#  if (not os.path.exists(output) or
#      (os.path.exists(source) and
#       os.path.getmtime(source) > os.path.getmtime(output))):
#    print('Generating %s...' % output)
#
#    if not os.path.exists(source):
#      sys.stderr.write("Can't find required file: %s\n" % source)
#      sys.exit(-1)
#
#    if protoc is None:
#      sys.stderr.write(
#          'protoc is not installed nor found in ../src.  Please compile it '
#          'or install the binary package.\n')
#      sys.exit(-1)
#
#    protoc_command = [protoc, '-I../src', '-I.', '--python_out=.', source]
#    if subprocess.call(protoc_command) != 0:
#      sys.exit(-1)
#
#class BuildPyCmd(_build_py):
#
#  def run(self):
#    GenProto('protonium/protonium.proto')
#
#    _build_py.run(self)

setup(
    name = 'protonium',
    version = '0.0.1',

    packages = [
        'protonium',
        'protonium.include',
    ],

    package_data = {
        'protonium': ['templates/*.j2'],
        'protonium.include': ['*.h', '*/*.h', '*/*/*.h'],
    },

    entry_points = {
        'console_scripts': [
            'protonium=protonium.cli:cli',
            'protoc-gen-protonium=protonium.generator:generate',
            'protoc-gen-protonium_python=protonium.generator:generate_python',
        ],
    },

    

    #cmdclass = {
    #    'build_py': BuildPyCmd,
    #}
)