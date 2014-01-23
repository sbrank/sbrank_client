from cx_Freeze import setup, Executable, sys
import sc2reader

# Dependencies are automatically detected, but it might need
# fine tuning.
buildOptions = dict(packages = [], excludes = [])

base = 'Console'

executables = [
    Executable('extractreplay.py', base=base, targetName = 'extractreplay.exe')
]

setup(name='sc2replayextractor',
      version = '0.1',
      description = 'SC2 Replay Extractor',
      options = dict(build_exe = buildOptions),
      executables = executables)
