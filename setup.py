# Copyright (c) 2019 CNES
#
# All rights reserved. Use of this source code is governed by a
# BSD-style license that can be found in the LICENSE file.
import datetime
import distutils.command.build
import pathlib
import platform
import re
import setuptools
import setuptools.command.build_ext
import setuptools.command.install
import subprocess
import os
import sys
import sysconfig

# Check Python requirement
MAJOR = sys.version_info[0]
MINOR = sys.version_info[1]
if not (MAJOR >= 3 and MINOR >= 6):
    raise RuntimeError("Python %d.%d is not supported, "
                       "you need at least Python 3.6." % (MAJOR, MINOR))


def execute(cmd):
    """Executes a command and returns the lines displayed on the standard
    output"""
    process = subprocess.Popen(cmd,
                               shell=True,
                               stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE)
    return process.stdout.read().decode()


def revision():
    """Returns the software version"""
    cwd = pathlib.Path().absolute()
    module = os.path.join(cwd, 'src', 'pyinterp', 'version.py')
    stdout = execute("git describe --tags --dirty --long --always").strip()
    pattern = re.compile(r'([\w\d\.]+)-(\d+)-g([\w\d]+)(?:-(dirty))?')
    match = pattern.search(stdout)

    # If the information is unavailable (execution of this function outside the
    # development environment), file creation is not possible
    if not stdout:
        pattern = re.compile(r'\s+result = "(.*)"')
        with open(module, "r") as stream:
            for line in stream:
                match = pattern.search(line)
                if match:
                    return match.group(1)
        raise AssertionError()

    # No tag already registred
    if match is None:
        pattern = re.compile(r'([\w\d]+)(?:-(dirty))?')
        match = pattern.search(stdout)
        version = "0.0"
        sha1 = match.group(1)
    else:
        version = match.group(1)
        sha1 = match.group(3)

    stdout = execute("git log  %s -1 --format=\"%%H %%at\"" % sha1)
    stdout = stdout.strip().split()
    date = datetime.datetime.utcfromtimestamp(int(stdout[1]))
    sha1 = stdout[0]

    # Updating the version number description in "meta.yaml"
    meta = os.path.join(cwd, 'conda', 'meta.yaml')
    with open(meta, "r") as stream:
        lines = stream.readlines()
    pattern = re.compile(r'\s+version:\s+(.*)')

    for idx, line in enumerate(lines):
        match = pattern.search(line)
        if match is not None:
            lines[idx] = '  version: %s\n' % version

    with open(meta, "w") as stream:
        stream.write("".join(lines))

    # Updating the version number description for sphinx
    conf = os.path.join(cwd, 'docs', 'source', 'conf.py')
    with open(conf, "r") as stream:
        lines = stream.readlines()
    pattern = re.compile(r'(\w+)\s+=\s+(.*)')

    for idx, line in enumerate(lines):
        match = pattern.search(line)
        if match is not None:
            if match.group(1) == 'version':
                lines[idx] = "version = %r\n" % version
            elif match.group(1) == 'release':
                lines[idx] = "release = %r\n" % version
            elif match.group(1) == 'copyright':
                lines[idx] = "copyright = '(%s, CNES/CLS)'\n" % date.year

    with open(conf, "w") as stream:
        stream.write("".join(lines))

    # Finally, write the file containing the version number.
    with open(module, 'w') as handler:
        handler.write('''"""
Get software version information
================================
"""


def release(full: bool = False) -> str:
    """Returns the software version number"""
    # {sha1}
    result = "{version}"
    if full:
        result += " ({date})"
    return result
'''.format(sha1=sha1, version=version, date=date.strftime("%d %B %Y")))
    return version


class CMakeExtension(setuptools.Extension):
    """Python extension to build"""

    def __init__(self, name):
        super(CMakeExtension, self).__init__(name, sources=[])


class BuildExt(setuptools.command.build_ext.build_ext):
    """Build the Python extension using cmake"""

    #: Preferred C++ compiler
    CXX_COMPILER = None

    #: Preferred BOOST root
    BOOST_ROOT = None

    #: Preferred GSL root
    GSL_ROOT = None

    #: Preferred Eigen root
    EIGEN3_INCLUDE_DIR = None

    def run(self):
        """A command's raison d'etre: carry out the action"""
        for ext in self.extensions:
            self.build_cmake(ext)
        super().run()

    @staticmethod
    def gsl():
        """Get the default boost path in Anaconda's environnement."""
        gsl_root = sys.prefix
        if os.path.exists(os.path.join(gsl_root, "include", "gsl")):
            return "-DGSL_ROOT_DIR=" + gsl_root
        gsl_root = os.path.join(sys.prefix, "Library")
        if not os.path.exists(os.path.join(gsl_root, "include", "gsl")):
            raise RuntimeError(
                "Unable to find the GSL library in the conda distribution "
                "used.")
        return "-DGSL_ROOT_DIR=" + gsl_root

    @staticmethod
    def boost():
        """Get the default boost path in Anaconda's environnement."""
        boost_root = sys.prefix
        if os.path.exists(os.path.join(boost_root, "include", "boost")):
            return "-DBOOST_ROOT=" + boost_root
        boost_root = os.path.join(sys.prefix, "Library", "include")
        if not os.path.exists(boost_root):
            raise RuntimeError(
                "Unable to find the Boost library in the conda distribution "
                "used.")
        return "-DBoost_INCLUDE_DIR=" + boost_root

    @staticmethod
    def eigen():
        """Get the default Eigen3 path in Anaconda's environnement."""
        eigen_include_dir = os.path.join(sys.prefix, "include", "eigen3")
        if os.path.exists(eigen_include_dir):
            return "-DEIGEN3_INCLUDE_DIR=" + eigen_include_dir
        eigen_include_dir = os.path.join(sys.prefix, "Library", "include")
        if not os.path.exists(eigen_include_dir):
            raise RuntimeError(
                "Unable to find the Eigen3 library in the conda distribution "
                "used.")
        return "-DEIGEN3_INCLUDE_DIR=" + eigen_include_dir

    @staticmethod
    def is_conda():
        """Detect if the Python interpreter is part of a conda distribution."""
        result = os.path.exists(os.path.join(sys.prefix, 'conda-meta'))
        if not result:
            try:
                import conda
            except:
                result = False
            else:
                result = True
        return result

    def build_cmake(self, ext):
        """Execute cmake to build the Python extension"""
        cwd = pathlib.Path().absolute()

        # These dirs will be created in build_py, so if you don't have
        # any python sources to bundle, the dirs will be missing
        build_temp = pathlib.Path(self.build_temp)
        build_temp.mkdir(parents=True, exist_ok=True)
        extdir = pathlib.Path(self.get_ext_fullpath(
            ext.name)).absolute().parent

        cfg = 'Debug' if self.debug else 'Release'

        cmake_args = [
            "-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=" + str(extdir),
            "-DPYTHON_EXECUTABLE=" + sys.executable
        ]

        if self.CXX_COMPILER is not None:
            cmake_args.append("-DCMAKE_CXX_COMPILER=" + self.CXX_COMPILER)

        is_conda = self.is_conda()

        if self.BOOST_ROOT is not None:
            cmake_args.append("-DBOOST_ROOT=" + self.BOOST_ROOT)
        elif is_conda:
            cmake_args.append(self.boost())

        if self.GSL_ROOT is not None:
            cmake_args.append("-DGSL_ROOT_DIR=" + self.GSL_ROOT)
        elif is_conda:
            cmake_args.append(self.gsl())

        if self.EIGEN3_INCLUDE_DIR is not None:
            cmake_args.append("-DEIGEN3_INCLUDE_DIR=" +
                              self.EIGEN3_INCLUDE_DIR)
        elif is_conda:
            cmake_args.append(self.eigen())

        build_args = ['--config', cfg]

        if platform.system() != 'Windows':
            build_args += ['--', '-j%d' % os.cpu_count()]
            cmake_args += ['-DCMAKE_BUILD_TYPE=' + cfg]
            if platform.system() == 'Darwin':
                cmake_args += ['-DCMAKE_OSX_DEPLOYMENT_TARGET=10.14']
        else:
            cmake_args += [
                '-DCMAKE_GENERATOR_PLATFORM=x64',
                '-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{}={}'.format(
                    cfg.upper(), extdir)
            ]
            build_args += ['--', '/m']
            if self.verbose:
                build_args += ['/verbosity:n']

        if self.verbose:
            build_args.insert(0, "--verbose")

        os.chdir(str(build_temp))
        self.spawn(['cmake', str(cwd)] + cmake_args)
        if not self.dry_run:
            self.spawn(['cmake', '--build', '.', '--target', 'core'] +
                       build_args)
        os.chdir(str(cwd))


class Build(distutils.command.build.build):
    """Build everything needed to install"""
    user_options = distutils.command.build.build.user_options
    user_options += [
        ('boost-root=', 'u', 'Preferred Boost installation prefix'),
        ('gsl-root=', 'g', 'Preferred GSL installation prefix'),
        ('eigen-root=', 'e', 'Preferred Eigen3 include directory'),
        ('cxx-compiler=', 'x', 'Preferred C++ compiler')
    ]

    def initialize_options(self):
        """Set default values for all the options that this command supports"""
        super().initialize_options()
        self.boost_root = None
        self.cxx_compiler = None
        self.eigen_root = None
        self.gsl_root = None

    def run(self):
        """A command's raison d'etre: carry out the action"""
        if self.boost_root is not None:
            BuildExt.BOOST_ROOT = self.boost_root
        if self.cxx_compiler is not None:
            BuildExt.CXX_COMPILER = self.cxx_compiler
        if self.gsl_root is not None:
            BuildExt.GSL_ROOT = self.gsl_root
        if self.eigen_root is not None:
            BuildExt.EIGEN3_INCLUDE_DIR = self.eigen_root
        super().run()


def main():
    setuptools.setup(
        name='pyinterp',
        version=revision(),
        classifiers=[
            "Development Status :: 3 - Alpha",
            "Topic :: Scientific/Engineering :: Physics",
            "License :: OSI Approved :: BSD License",
            "Natural Language :: English", "Operating System :: POSIX",
            "Operating System :: MacOS",
            "Operating System :: Microsoft :: Windows",
            "Programming Language :: Python :: 3.6",
            "Programming Language :: Python :: 3.7"
        ],
        description='Interpolation of geo-referenced data for Python.',
        url='https://github.com/CNES/pangeo-pyinterp',
        author='CNES/CLS',
        license="BSD License",
        ext_modules=[CMakeExtension(name="pyinterp.core")],
        package_dir={'': 'src'},
        packages=setuptools.find_namespace_packages(where='src',
                                                    exclude=['*core*']),
        install_requires=["numpy", "xarray"],
        tests_require=["netCDF4", "numpy", "xarray"],
        cmdclass={
            'build': Build,
            'build_ext': BuildExt
        },
        zip_safe=False)


if __name__ == "__main__":
    main()
