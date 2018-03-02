sortmerna
=========

[![Build Status](https://travis-ci.org/biocore/sortmerna.png?branch=master)](https://travis-ci.org/biocore/sortmerna)

SortMeRNA is a local sequence alignment tool for filtering, mapping and clustering.

The core algorithm is based on approximate seeds and allows for sensitive analysis of NGS reads.
The main application of SortMeRNA is filtering rRNA from metatranscriptomic data.
SortMeRNA takes as input a file of reads (fasta or fastq format) and one or multiple
rRNA database file(s), and sorts apart aligned and rejected reads into two files specified by the user.
Additional applications include clustering and taxonomy assignation available through QIIME v1.9.1
(http://qiime.org). SortMeRNA works with Illumina, Ion Torrent and PacBio data, and can produce SAM and
BLAST-like alignments.

Visit http://bioinfo.lifl.fr/RNA/sortmerna/ for more information.


# Table of Contents
* [Support](#support)
* [Documentation](#documentation)
* [Getting Started](#getting-started)
* [Compilation](#sortmerna-compilation)
	* [Linux OS](#linux-os)
	* [Mac OS](#mac-os)
	* [Windows OS](#windows-os)
* [Tests](#tests)
* [Third-party libraries](#third-party-libraries)
* [Wrappers and packages](#wrappers-and-packages)
	* [Galaxy](#galaxy)
	* [Debian](#debian)
	* [GNU Guix](#gnu-guix)
	* [QIIME](#qiime)
* [Taxonomies](#taxonomies)
* [Citation](#citation)
* [Contributors](#contributors)
* [References](#references)


# Support
For questions and comments, please use the SortMeRNA [forum](https://groups.google.com/forum/#!forum/sortmerna).

  
# Documentation

If you have [Doxygen](http://www.stack.nl/~dimitri/doxygen/) installed, you can generate the documentation
by modifying the following lines in ```doxygen_configure.txt```:

```
INPUT = /path/to/sortmerna/include /path/to/sortmerna/src
IMAGE_PATH = /path/to/sortmerna/algorithm
```

and running the following command:

```
doxygen doxygen_configure.txt
```

This command will generate a folder ```html``` in the directory from which the
command was run.


# Getting Started

SortMeRNA can be built and run on Windows, Linux, and Mac.

There are 3 methods to install SortMeRNA:

1. [GitHub repository](https://github.com/biocore/sortmerna) development version (master branch)
...* [Installation instructions](#sortmerna-compilation)
2. [GitHub releases](https://github.com/biocore/sortmerna/releases) (tar balls, zip)
...* [Installation instructions Linux](#linux-os)
...* [Installation instructions Mac OS](#mac-os)
...* [Installation instructions Windows OS](#windows-os)
3. [BioInfo releases](http://bioinfo.lifl.fr/RNA/sortmerna/) (tar balls including compiled binaries)

Option (3) is the simplest, as it provides access to pre-compiled binaries to various OS.

# SortMeRNA Compilation

CMake is used for generating the build files and should be installed prior the build.
CMake distributions are available for all major operating systems.
Please visit [CMake project website](https://cmake.org/) for download and installation instructions.

## Linux OS

We tested the build on Ubuntu 16.04 LTS Xenial with GCC 5.4.0

(1) Install GCC if not already installed. SortmeRNA is C++14 compliant, so the GCC needs to be fairly new e.g. 5.4.0 works OK.

	```bash
	gcc --version
		gcc (Ubuntu 5.4.0-6ubuntu1~16.04.4) 5.4.0 20160609
	```

(2) Install pre-requisites (CMake, Git, Zlib, RocksDB, RapidJson)

	```
	sudo apt update
	sudo apt install cmake
	sudo apt install git
	suod apt install zlib
	sudo apt install rocksdb
	sudo apt install rapidjson
	```
	
(3) Clone the Git repository

	```
	git clone https://github.com/biocore/sortmerna.git
	```
	
(2) Generate the build files using CMake:

	```bash
	mkdir -p $SMR_HOME/build/Release
	pushd $SMR_HOME/build/Release
	cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DEXTRA_CXX_FLAGS_RELEASE="-pthread" ../..
	```

NOTE: `$SMR_HOME` is the top directory where sortmerna code (e.g. git repo) is located.

The above commands will perform necessary system check-ups, dependencies, and generate Makefile.

(3) Compile and build executables:

	```bash
	make
	```

The binaries are created in `$SMR_HOME/build/Release/src/indexdb` and `$SMR_HOME/build/Release/src/sortmerna`
Simply add the build binaries to the PATH e.g.
`export PATH="$SMR_HOME/build/Release/src/indexdb:$SMR_HOME/build/Release/src/sortmerna:$PATH"`


## Mac OS

We tested the build on macOS 10.13 High Sierra (64-bit).
We recommend the Homebrew - an excellent packager for Mac [1], which has all the latest packages required to build SortmeRNA.
The build can be performed using either Clang or GCC.

(1) Install Homebrew:

```bash
/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)" [1]

brew --version
brew help
```
	
(2) Install pre-requisites (CMake, Git, Zlib, RocksDB, RapidJson)

```bash
brew install cmake
brew install git
brew install zlib
brew install rocksdb
brew install rapidjson
```

(3) Clone the GIt repository

```
git clone https://github.com/biocore/sortmerna.git
```

(4) Generate the build files:

```bash
mkdir -p $SMR_HOME/build/Release
pushd $SMR_HOME/build/Release
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DEXTRA_CXX_FLAGS_RELEASE="-pthread" ../..
	-- The CXX compiler identification is AppleClang 9.0.0.9000039
	-- The C compiler identification is AppleClang 9.0.0.9000039
	-- Check for working CXX compiler: /Library/Developer/CommandLineTools/usr/bin/c++
	-- Check for working CXX compiler: /Library/Developer/CommandLineTools/usr/bin/c++ -- works
	-- Detecting CXX compiler ABI info
	-- Detecting CXX compiler ABI info - done
	-- Detecting CXX compile features
	-- Detecting CXX compile features - done
	-- Check for working C compiler: /Library/Developer/CommandLineTools/usr/bin/cc
	-- Check for working C compiler: /Library/Developer/CommandLineTools/usr/bin/cc -- works
	-- Detecting C compiler ABI info
	-- Detecting C compiler ABI info - done
	-- Detecting C compile features
	-- Detecting C compile features - done
	CMAKE_CXX_COMPILER_ID = AppleClang
	CMAKE_CONFIGURATION_TYPES =
	CMAKE_CXX_FLAGS_RELEASE: -O3 -DNDEBUG
	EXTRA_CXX_FLAGS_RELEASE: -pthread
	Cloning into 'concurrentqueue'...
	Checking out files: 100% (1613/1613), done.
	-- Configuring done
	-- Generating done
	-- Build files have been written to: /Users/bc/sortmerna/build/Release
```

Note: `$SMR_HOME` is the top directory where sortmerna code (e.g. git repo) is located.

CMake will perform necessary system check-ups, dependencies, and generate Makefile.

(5) Compile and build executables:

```bash
make
```

The binaries are created in `$SMR_HOME/build/Release/src/indexdb` and `$SMR_HOME/build/Release/src/sortmerna`
Simply add the build binaries to the PATH e.g.
`export PATH="$SMR_HOME/build/Release/src/indexdb:$SMR_HOME/build/Release/src/sortmerna:$PATH"`


### Set Clang compiler for Mac OS

(1) Check if you have Clang installed:

```bash
clang --version
```

(2a) If Clang is installed, set your compiler to Clang:

```bash
export CC=clang
export CXX=clang++
```

(2b) If Clang is not installed, see [Clang for Mac OS](#clang-for-mac-os)
for installation instructions.

### Set GCC compiler for Mac OS

(1) Check if you have GCC installed:

```bash
gcc --version
```

(2a) If GCC is installed, set your compiler to GCC:

```bash
export CC=gcc-mp-5.4
export CXX=g++-mp-5.4
```

(2b) If GCC is not installed, it can be installed through Homebrew or MacPorts.

```
brew tap homebrew/versions
brew install [flags] gcc54
```

To list available flags
```
brew options gcc54
```

Clang for Mac OS
----------------

Installing Xcode (free through the App Store) and Xcode command line tools will automatically 
install the latest version of Clang supported with Xcode. 

After installing Xcode, the Xcode command line tools may be installed via:

Xcode -> Preferences -> Downloads

Under "Components", click to install "Command Line Tools"


## Windows OS

MS Visual Studio Community edition and CMake for Windows are required for building SortMeRNA.

We tested the build using `Visual Studio 15 2017 Win64` and `Visual Studio 14 2015 Win64`

(1) Download and Install VS Community edition from [Visual Studio community website](https://www.visualstudio.com/vs/community/)

(2) Install CMake

CMake can be installed using either Windows Installer or binaries from archive.
Download binary distributions from [here](https://cmake.org/download/)

If you choose portable binaries (not the installer) e.g. cmake-3.11.0-rc1-win64-x64.zip,
just download and extract the archive in a directory of your choice e.g.

```
C:\libs\cmake-3.11.0-rc1-win64-x64\
	bin\
	doc\
	man\
	share\
```

The `bin` directory above contains `cmake.exe` and `cmake-gui.exe`. Add the `bin` directory to your PATH
Start `cmd` and

```
set PATH=C:\libs\cmake-3.11.0-rc1-win64-x64\bin;%PATH%
cmake --version
cmake-gui
```

(3) Install Git for Windows

Download binary distribution either portable or the installer from [here](https://git-scm.com/download/win)

The portable distribution is a self-extracting archive that can be installed in a directory of your choice e.g.

```
C:\libs\git-2.16.2-64\
	bin\
	cmd\
	dev\
	etc\
	mingw64\
	tmp\
	usr\
```
You can use either `bash.exe` or native Windows CMD `cmd.exe`.

If you choose to work with CMD, add the following to your path:

```
set GIT_HOME=C:\libs\git-2.16.2-64
set PATH=%GIT_HOME%\bin;%GIT_HOME%\usr\bin;%GIT_HOME%\mingw64\bin;%PATH%

git --version
```

(4) Clone the GIt repository

```
git clone https://github.com/biocore/sortmerna.git
```

(5) Prepare the build files:

On Windows we recommend using the `cmake-gui` utility.
Either navigate to CMake installation directory (using Windows Explorer) and double-click
`cmake-gui`, or launch it from command line as shown below:

```
set PATH=C:\libs\cmake-3.11.0-rc1-win64-x64\bin;%PATH%
cmake-gui
```

In the CMake GUI
 - click `Browse source` button and navigate to the directory where Sortmerna sources are located (SMR_HOME).
 - click `Browse Build` and navigate to the directory where to build the binaries e.g. %SMR_HOME%\build
 - at the prompt select the Generator from the list e.g. "Visual Studio 15 2017 Win64"
 - click `Configure`
 - Set the following variables:
   - ZLIB_INCLUDE_DIR=%SMR_HOME%/3rdparty/zlib
   - ZLIB_LIB_DEBUG=%SMR_HOME%/3rdparty/zlib/build/Debug
   - ZLIB_LIB_RELEASE=%SMR_HOME%/3rdparty/zlib/build/Release
   - ROCKSDB_INCLUDE_DIR=%SMR_HOME%/3rdparty/rocksdb/include
   - ROCKSDB_LIB_DEBUG=%SMR_HOME%/3rdparty/rocksdb/build/Debug
   - ROCKSDB_LIB_RELEASE=%SMR_HOME%/3rdparty/rocksdb/build/Release
 - click `Configure` again
 - click `Generate` if all variables were set OK (no red background)

The `Generate` generates VS project files in `%SMR_HOME%\build\` directory.
`%SMR_HOME%` is the top directory where SortMeRNA source distribution (e.g. Git repo) is installed.

(6) Configure and build Zlib library

When Cmake-gui `Configure` is run it downloads required 3rd party source packages into `%SMR_HOME%\3rdparty\` directory.

In Cmake-gui:
- click `Browse Source...` and select `%SMR_HOME%\3rdparty\zlib\`
- click `Browse Build...` and select `%SMR_HOME%\3rdparty\zlib\build\` (confirm to create the `build` directory if not already exists)
- click `Configure` and set the required variables or accept defaults
- click `Generate`

In Visual Studio
- `File -> Open -> Project/Solution` and select `%SMR_HOME%\3rdparty\zlib\build\zlib.sln`
- In Solution Explorer right-click `ALL_BUILD` and select `build` from drop-down menu

(7) COnfigure and build RockDB library

In Cmake-gui:
- click `Browse Source...` and select `%SMR_HOME%\3rdparty\rocksdb\`
- click `Browse Build...` and select `%SMR_HOME%\3rdparty\rocksdb\build\` (confirm to create the `build` directory if not already exists)
- click `Configure` and set the following variables:
  - Ungrouped Entries
    - PORTABLE (checkbox)
	- GIT_EXECUTABLE (select path to `git.exe` e.g. `C:/libs/git-2.16.2-64/bin/git.exe`
  - WITH
    - WITH_MD_LIBRARY
	- WITH_ZLIB
	- Accept defaults for the rest
- click `Generate`

In Visual Studio
- `File -> Open -> Project/Solution` and select `%SMR_HOME%\3rdparty\rocksdb\build\rocksdb.sln`
- In Solution Explorer right-click `ALL_BUILD` and select `build` from drop-down menu

(8) Build SormeRNA

In Visual Studio:
- `File -> Open -> Project/Solution .. open %SMR_HOME%\build\sortmerna.sln`
- Select desired build type: `Release | Debug | RelWithDebInfo | MinSizeRel`.
- In Solution explorer right-click `ALL_BUILD' and select `build` in drop-down menu.

Depending on the build type the binaries are generated in 
`%SMR_HOME%\build\src\sortmerna\Release` (or `Debug | RelWithDebInfo | MinSizeRel`).

(9) Add sortmerna executables to PATH

```
set PATH=%SMR_HOME%\build\src\indexdb\Release;%SMR_HOME%\build\src\sortmerna\Release;%PATH%
```

Tests
=====

Python code is provided for running tests in $SRM_HOME/tests (%SRM_HOME%\tests) and requires Python 3.5 or higher.

Tests can be run with the following command:
```
python ./tests/test_sortmerna.py
python ./tests/test_sortmerna_zlib.py
```
Make sure the ```data``` folder is in the same directory as ```test_sortmerna.py```

Users require [scikit-bio](https://github.com/biocore/scikit-bio) 0.5.0 to run the tests.


Third-party libraries
=====================
Various features in SortMeRNA are dependent on third-party libraries, including:
* [ALP](http://www.ncbi.nlm.nih.gov/CBBresearch/Spouge/html_ncbi/html/software/program.html?uid=6): computes statistical parameters for Gumbel distribution (K and Lambda)
* [CMPH](http://cmph.sourceforge.net): C Minimal Perfect Hashing Library
* [Zlib](https://github.com/madler/zlib): reading compressed Reads files
* [RocksDB](https://github.com/facebook/rocksdb): storage for SortmeRNA alignment results
* [RapidJson](https://github.com/Tencent/rapidjson): serialization of Reads objects to store in RocksDB
* [Concurrent Queue](https://github.com/cameron314/concurrentqueue): Lockless buffer for Reads accessed from multiple processing threads

Wrappers and Packages
=====================

Galaxy
------

Thanks to Björn Grüning and Nicola Soranzo, an up-to-date Galaxy wrapper exists for SortMeRNA.
Please visit Björn's [github page](https://github.com/bgruening/galaxytools/tree/master/tools/rna_tools/sortmerna) for installation.

Debian
------

Thanks to the [Debian Med](https://www.debian.org/devel/debian-med/) team, SortMeRNA 2.0 is now a package in Debian.
Thanks to Andreas Tille for the sortmerna and indexdb_rna man pages (version 2.0).
These have been updated for 2.1 in the master repository.

GNU Guix
--------

Thanks to Ben Woodcroft for adding SortMeRNA 2.1 to GNU Guix, find the package [here](https://www.gnu.org/software/guix/packages/).

QIIME
-----

SortMeRNA 2.0 can be used in [QIIME](http://qiime.org)'s [pick_closed_reference_otus.py](http://qiime.org/scripts/pick_closed_reference_otus.html),
[pick_open_reference_otus.py](http://qiime.org/scripts/pick_open_reference_otus.html) and [assign_taxonomy.py](http://qiime.org/scripts/assign_taxonomy.html) scripts.

Note: At the moment, only 2.0 is compatible with QIIME.

Taxonomies
==========

The folder `rRNA_databases/silva_ids_acc_tax.tar.gz` contains SILVA taxonomy strings (extracted from XML file generated by ARB)
for each of the reference sequences in the representative databases. The format of the files is three tab-separated columns,
the first being the reference sequence ID, the second being the accession number and the final column is the taxonomy.

Citation
========

If you use SortMeRNA, please cite:
Kopylova E., Noé L. and Touzet H., "SortMeRNA: Fast and accurate filtering of ribosomal RNAs in metatranscriptomic data", Bioinformatics (2012), doi: 10.1093/bioinformatics/bts611.

Contributors
============
See [AUTHORS](./AUTHORS) for a list of contributors to this project.

References
==========

1. Homebrew 
	- [home](https://brew.sh/)
	- [github](https://github.com/Homebrew)
	