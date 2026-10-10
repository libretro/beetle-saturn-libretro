#!/usr/bin/env python3
"""Build and run deterministic thread/codec tests, with no BIOS or game.

Run from an environment containing GNU make, GCC and Python. The fixture
writer uses an independent Zstd shared library (test-only dependency).
Example: python tools/check_lockfree.py --zstd-library /path/to/libzstd
Pass --no-threading to exercise the synchronous renderer and CD path.
"""
import argparse
import os
from pathlib import Path
import shlex
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parent.parent
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--cc', default='gcc')
parser.add_argument('--make', default='make')
parser.add_argument('--platform', default='win' if os.name == 'nt' else 'unix')
parser.add_argument('--jobs', default='4')
parser.add_argument('--zstd-library')
parser.add_argument('--no-threading', action='store_true')
parser.add_argument('--frontend-eventcount-source', type=Path,
                    help='Unchanged RetroArch retro_eventcount.c for Windows integration')
args = parser.parse_args()

def run(command, **kwargs):
    return subprocess.run(command, cwd=ROOT, check=True, **kwargs)

make = [args.make, '--no-print-directory', 'platform=' + args.platform,
        'CC=' + args.cc, 'CXX=g++']
if args.no_threading:
    make += ['NEED_THREADING=0']
# A clean graph prevents mixing objects built with different feature flags.
run(make + ['clean'])
run(make + ['-j' + args.jobs])

def variable(name):
    rule = 'saturn_test_config:;@echo $(' + name + ')'
    return run(make + ['-s', '--eval=' + rule, 'saturn_test_config'],
               stdout=subprocess.PIPE, text=True).stdout.strip()

objects = shlex.split(variable('OBJECTS'))
flags = shlex.split(variable('CFLAGS')) + ['-UNDEBUG']
libraries = ['-lwinmm', '-static-libgcc'] if args.platform == 'win' else ['-pthread', '-lm']
thread_sources = ['mednafen/thread_eventcount.c', 'mednafen/thread_queue.c',
                  'libretro-common/queues/retro_spsc.c',
                  'libretro-common/rthreads/rthreads.c',
                  'libretro-common/features/features_cpu.c',
                  'libretro-common/time/rtime.c']
suffix = '.exe' if os.name == 'nt' else ''

with tempfile.TemporaryDirectory(prefix='saturn-tests-') as directory:
    scratch = Path(directory)
    def compile_test(name, sources, extra=()):
        executable = scratch / (name + suffix)
        run([args.cc] + flags + list(extra) + ['tools/' + name + '.c'] +
            sources + ['-o', str(executable)] + libraries)
        return executable

    cd_sources = ['mednafen/cdrom/cdromif.c'] + thread_sources
    if args.no_threading:
        cd_sources = ['mednafen/cdrom/cdromif.c',
                      'libretro-common/features/features_cpu.c',
                      'libretro-common/time/rtime.c']
    cd = compile_test('cdrom_thread_test', cd_sources)
    run([str(cd)], timeout=60)
    if not args.no_threading:
        run([str(cd), 'slow-io'], timeout=60)
        event = compile_test('eventcount_test', thread_sources)
        run([str(event)], timeout=60)
        if args.platform == 'win':
            isolation = compile_test('windows_eventcount_test', thread_sources,
                                     ['-DSATURN_EVENTCOUNT_SPIN_ITERS=0'])
            run([str(isolation)], timeout=60)
            if args.frontend_eventcount_source:
                isolation = compile_test('windows_eventcount_test',
                    thread_sources + [str(args.frontend_eventcount_source.resolve())],
                    ['-DSATURN_EVENTCOUNT_SPIN_ITERS=0', '-DSATURN_TEST_FRONTEND'])
                run([str(isolation)], timeout=60)
        event = compile_test('eventcount_test', thread_sources,
                             ['-DSATURN_EVENTCOUNT_FORCE_SLEEP'])
        run([str(event), 'short'], timeout=60)
    renderer = compile_test('renderer_thread_test',
                            [p for p in objects if Path(p).name != 'vdp2_render.o'])
    run([str(renderer)], timeout=60)
    io = compile_test('io_codec_test',
                     [p for p in objects if Path(p).name != 'CDAccess_CHD.o'])
    fixtures = scratch / 'fixtures'
    command = [sys.executable, 'tools/make_io_fixtures.py', str(fixtures)]
    if args.zstd_library:
        command += ['--zstd-library', args.zstd_library]
    run(command)
    for chd in sorted(fixtures.glob('*.chd')):
        run([str(io), str(chd), str(fixtures / 'roms.zip')], timeout=60)
        truncated = scratch / 'truncated.chd'
        truncated.write_bytes(chd.read_bytes()[:-64])
        result = subprocess.run([str(io), str(truncated), str(fixtures / 'roms.zip')],
                                cwd=ROOT, timeout=60, stdout=subprocess.PIPE, text=True)
        if result.returncode not in (2, 5):
            raise RuntimeError('Truncated CHD was not rejected: ' + chd.name)
    print('Truncated CHD ranges rejected for all codecs.')
print('All thread/codec checks passed.')
