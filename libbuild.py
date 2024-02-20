#-*- coding: utf-8 -*-

from sys import argv
from os import pathsep, environ, system
from glob import glob
from subprocess import Popen
from hashlib import sha256

import os.path
import sys

_cache_file = None
_cache = {}
_quiet = False


def var(name, value):
    result = environ.get(name, value)
    globals()[name] = result
    return result


def arrvar(name, value, separator=None):
    if separator is None:
        separator = pathsep

    result = environ.get(name)

    if result is None:
        result = value
    else:
        result = result.spltit(separator)

    globals()[name] = result
    return result


def wildcard(*tuple_path, prefix=None, suffix=None):
    path = list(tuple_path)

    if len(path) == 0:
        return []

    if prefix is None:
        prefix = ''

    if suffix is None:
        suffix = ''

    path[0] = prefix + path[0]
    path[-1] += suffix

    return glob(os.path.join(*path), recursive=True)


def patsubst(original, from_prefix=None, to_prefix=None, from_suffix=None, to_suffix=None):
    from_prefix = '' if from_prefix is None else from_prefix
    to_prefix = '' if to_prefix is None else to_prefix
    from_suffix = '' if from_suffix is None else from_suffix
    to_suffix = '' if to_suffix is None else to_suffix

    result = []

    for orig in original:
        if len(from_prefix) and orig.startswith(from_prefix):
            orig = to_prefix + orig[len(from_prefix):]

        if len(from_suffix) and orig.endswith(from_suffix):
            orig = orig[:-len(from_suffix)] + to_suffix

        result.append(orig)

    return result


def strarr(arr, separator=' '):
    return separator.join(arr)


def _qprint(*args, **kwargs):
    if not _quiet:
        print(*args, **kwargs)


def cmd(com, handle_error = sys.exit, handle_return_code = lambda code: code):
    _qprint('>', com)
    code = system(com)

    if code != 0:
        handle_error(code)

    handle_return_code(code)


def hash(data: bytes) -> bytes:
    return sha256(data).digest()


_build_dict = {}
_targets = {}


def build_func(artifacts, sources, build_func):
    if len(artifacts) != len(sources):
        raise ValueError('Artifacts and sources lists must have same length')

    for i in range(len(artifacts)):
        source = sources[i]
        artifact = artifacts[i]
        _build_dict[artifacts[i]] = (lambda src, out: lambda: build_func(src, out))(source, artifact)


def _callback_build_target(artifact, requirements, build_func):
    for req in requirements:
        if req not in _build_dict:
            raise ValueError(f'No function to build "{req}"')

        _qprint(f'Building "{req}"')
        _build_dict[req]()

    build_func(requirements, artifact)


def build_target(target_name, artifact, requirements, build_func):
    _targets[target_name] = lambda: _callback_build_target(artifact, requirements, build_func)


def _callback_run_target(dependencies, run_func):
    for dep in dependencies:
        if dep not in _targets:
            raise ValueError(f'No function to build dependency target "{dep}"')

        _targets[dep]()

    run_func()


def run_target(target_name, dependencies, run_func):
    _targets[target_name] = lambda: _callback_run_target(dependencies, run_func)


def target(target_name, run_func):
    return run_target(target_name, [], run_func)


def exec_target(name):
    _targets[name]()


def build(custom_argv=None):
    if custom_argv is None:
        custom_argv = argv[1:]

    if len(custom_argv) == 0:
        if 'default' in _targets:
            custom_argv.append('default')
        else:
            raise ValueError('No target specified to build')

    for arg in custom_argv:
        if arg in _targets:
            _qprint(f'Running target "{arg}"')
            _targets[arg]()
        elif arg in _build_dict:
            _qprint(f'Building file "{arg}"')
            _build_dict[arg]()
        else:
            raise ValueError(f'{arg}: No such target or file to build')

    if len(_cache) > 0 and _cache_file is not None:
        save_cache()
        _qprint('Cache updated')


def run_file(*path, command=None):
    if command is None:
        proc = Popen(os.path.join('.', *path))
    else:
        print('Running:', command + ' ' + os.path.join(*path))
        proc = Popen([command, os.path.join(*path)], executable=command)

    proc.wait()
    _qprint('File runned')


def enable_cache(cache_path):
    global _cache_file, _cache
    _cache_file = cache_path
    _cache = {}
    load_cache()


def is_changed(file_str):
    file_path = file_str.encode('UTF-8')

    if file_path not in _cache:
        return True

    with open(file_path, 'rb') as file:
        file_hash = hash(file.read())

    return _cache[file_path] != file_hash


def update_cache(file_path):
    with open(file_path, 'rb') as file:
        _cache[file_path.encode('UTF-8')] = hash(file.read())


def load_cache():
    try:
        with open(_cache_file, 'rb') as file:
            for line in file.read().split(b'\n'):
                if len(line):
                    file_data = line.split(b'\0')
                    file_name = file_data[0]
                    file_hash = file_data[1]
                    _cache[file_name] = file_hash
    except (FileNotFoundError, IndexError) as exc:
        _qprint('Cache file is invalid or not exists:', exc)


def save_cache():
    if _cache_file is None:
        raise ValueError('Cache is disabled, but save_cache() was called')

    with open(_cache_file, 'wb') as file:
        for file_name, file_hash in _cache.items():
            file.write(file_name)
            file.write(b'\0')
            file.write(file_hash)
            file.write(b'\n')

        file.flush()


def list_cache() -> list[str]:
    arr = []

    for file_path, hash in _cache.items():
        arr.append(file_path.decode('UTF-8') + os.pathsep + hash.hex())

    return arr


def clear_cache():
    global _cache
    _cache = {}


def set_quiet_mode(new_mode: bool):
    global _quiet
    _quiet = new_mode


def main():
    raise Exception('This file is a library, so it cannot be runned directly')


if __name__ == '__main__':
    main()
