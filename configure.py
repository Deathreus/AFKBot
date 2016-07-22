#!/usr/bin/python2
# vim: set sts=2 ts=8 sw=2 tw=99 et:
API_VERSION = '2.1'

import sys
try:
  from ambuild2 import run
  if not run.HasAPI(API_VERSION):
    raise Exception()
except:
  sys.stderr.write('AMBuild {0} must be installed to build this project.\n'.format(API_VERSION))
  sys.stderr.write('http://www.alliedmods.net/ambuild\n')
  sys.exit(1)

builder = run.BuildParser(sourcePath = sys.path[0], api = API_VERSION)

builder.options.add_option('--hl2sdk-root', type=str, dest='hl2sdk_root', default=None, help='Root search folder for HL2SDKs')
builder.options.add_option('--mms-path', type=str, dest='mms_path', default=None, help='Path to Metamod:Source')
builder.options.add_option('--sm-path', type=str, dest='sm_path', default=None, help='Path to SourceMod')
builder.options.add_option('--enable-debug', action='store_const', const='1', dest='debug', help='Enable debugging symbols')
builder.options.add_option('--enable-optimize', action='store_const', const='1', dest='opt', help='Enable optimization')
builder.options.add_option('--enable-experimental', action='store_const', const='1', dest='experimental', help='Enable experimental/incomplete features')
builder.options.add_option('-s', '--sdks', default='all', dest='sdks', help='Build against specified SDKs; valid args are "all", "present", or comma-delimited list of engine names (default: %default)')

builder.Configure()
