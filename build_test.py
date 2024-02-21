#!/usr/bin/env python
#-*- coding: utf-8 -*-

from shutil import rmtree
from os import makedirs, mkdir
from libbuild import *
from os.path import dirname

NAME_LC = var('NAME_LC', 'main_test')
CC = var('CC', 'gcc')
CXX = var('CXX', 'g++')
GDB = var('GDB', 'gdb')
DEBUG_LEVEL = var('DEBUG_LEVEL', '3')
OPT_LEVEL = var('OPT_LEVEL', '0')
INCLUDES = arrvar('INCLUDES', ['include'])
INCLUDES = [f'-I{include}' for include in INCLUDES]
CFLAGS_STATIC = arrvar('CFLAGS_STATIC', ['-Wall', '-Wextra', '-Werror', f'-g{DEBUG_LEVEL}', f'-O{OPT_LEVEL}', '-std=c17', *INCLUDES])
CCFLAGS_STATIC = arrvar('CCFLAGS_STATIC', ['-Wall', '-Wextra', '-Werror', f'-g{DEBUG_LEVEL}', f'-O{OPT_LEVEL}', '-std=c++17', *INCLUDES, '-D_GLIBCXX_DEBUG'])
CFLAGS_EXE = arrvar('CFLAGS_EXE', ['-Wall', '-Wextra', '-Werror', f'-g{DEBUG_LEVEL}', f'-O{OPT_LEVEL}', '-std=c++17', *INCLUDES])

SRC_CC = wildcard('src', '**', '*', suffix='.c')
SRC_CXX = wildcard('src', '**', '*', suffix='.cpp')
SRC_CXX = [*[x for x in SRC_CXX if not x.endswith('main.cpp')], *wildcard('tests', '**', '*', suffix='.cpp')]

OBJ_CC = patsubst(SRC_CC, from_prefix='src', to_prefix='out_test', from_suffix='.c', to_suffix='.c.o')
OBJ_CXX = patsubst(patsubst(SRC_CXX, from_prefix='src', to_prefix='out_test', from_suffix='.cpp', to_suffix='.cpp.o'), from_prefix='tests', to_prefix='out_test')


def clean():
    rmtree('out_test', ignore_errors=True)
    mkdir('out_test')
    clear_cache()

    try:
        os.remove(NAME_LC)
    except FileNotFoundError:
        pass


changed_at_least_something: bool = False


def cc(com, src, path):
    global changed_at_least_something

    if is_changed(src):
        changed_at_least_something = True
        makedirs(dirname(path), exist_ok=True)
        cmd(com)
        update_cache(src)


def cc_exe(com, path):
    if changed_at_least_something:
        if len(path):
            try:
                makedirs(dirname(path), exist_ok=True)
            except FileNotFoundError:
                pass

        cmd(com)


set_quiet_mode(False)
build_func(OBJ_CC, SRC_CC, lambda source, artifact: cc(f'{CC} -o {artifact} -c {source} {strarr(CFLAGS_STATIC)}', source, artifact))
build_func(OBJ_CXX, SRC_CXX, lambda source, artifact: cc(f'{CXX} -o {artifact} -c {source} {strarr(CCFLAGS_STATIC)}', source, artifact))
build_target('build', NAME_LC, OBJ_CC + OBJ_CXX, lambda source, artifact: cc_exe(f'{CXX} -o {artifact} {strarr(source)} {strarr(CFLAGS_EXE)}', artifact))
run_target('run', ['build'], lambda: run_file(NAME_LC))
target('clear', clean)
target('clean', clean)
target('default', lambda: exec_target('run'))
run_target('debug', ['build'], lambda: run_file(NAME_LC, command=GDB))
enable_cache(os.path.join('out_test', 'cache'))


def main():
    build()


if __name__ == '__main__':
    main()
