# Ror use with building .so and installing with pip, use:
# from distutils.core import setup, Extension 
from setuptools import setup, Extension

moduleSharedHashFile = Extension('SharedHashFile',
                    define_macros = [('MAJOR_VERSION', '0'),
                                     ('MINOR_VERSION', '1')],
                    include_dirs = ['../../../src'],
                    extra_objects = ['obj_files/shf.o', 'obj_files/SharedHashFile.o', 'obj_files/murmurhash3.o' ],
                    #extra_objects = ['../../../release/shf.o', '../../../release/SharedHashFile.o', '../../../release/murmurhash3.o' ],
                    sources = ['SharedHashFile.cpp'])

setup (name = 'SharedHashFile',
       version = '0.1',
       description = 'Module to interact with a SharedHashFile Ipc mechanism.',
       author = 'Luke Woydziak',
       author_email = 'SharedHashFile@woydziak.com',
       url = 'https://github.com/simonhf/SharedHashFile',
       long_description = '''
Module to interact with a SharedHashFile Ipc mechanism.
''',
       ext_modules = [moduleSharedHashFile])
