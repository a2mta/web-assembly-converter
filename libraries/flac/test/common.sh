#  FLAC - Free Lossless Audio Codec
#  Copyright (C) 2001-2009  Josh Coalson
#  Copyright (C) 2011-2023  Xiph.Org Foundation
#
#  This file is part the FLAC project.  FLAC is comprised of several
#  components distributed under different licenses.  The codec libraries
#  are distributed under Xiph.Org's BSD-like license (see the file
#  COPYING.Xiph in this distribution).  All other programs, libraries, and
#  plugins are distributed under the GPL (see COPYING.GPL).  The documentation
#  is distributed under the Gnu FDL (see COPYING.FDL).  Each file in the
#  FLAC distribution contains at the top the terms under which it may be
#  distributed.
#
#  Since this particular file is relevant to all components of FLAC,
#  it may be distributed under the Xiph.Org license, which is the least
#  restrictive of those mentioned above.  See the file COPYING.Xiph in this
#  distribution.

export MALLOC_CHECK_=3
export MALLOC_PERTURB_=$((RANDOM % 255 + 1))

if [ -z "$1" ] ; then
	BUILD=debug
else
	BUILD="$1"
fi

LD_LIBRARY_PATH=../objs/$BUILD/lib:$LD_LIBRARY_PATH
LD_LIBRARY_PATH="$(pwd)/../objs/$BUILD/lib:$LD_LIBRARY_PATH"
LD_LIBRARY_PATH="$(pwd)/../src/libFLAC/.libs:$LD_LIBRARY_PATH"
LD_LIBRARY_PATH="$(pwd)/../src/share/getopt/.libs:$LD_LIBRARY_PATH"
LD_LIBRARY_PATH="$(pwd)/../src/share/grabbag/.libs:$LD_LIBRARY_PATH"
LD_LIBRARY_PATH="$(pwd)/../src/share/replaygain_analysis/.libs:$LD_LIBRARY_PATH"
LD_LIBRARY_PATH="$(pwd)/../src/share/replaygain_synthesis/.libs:$LD_LIBRARY_PATH"
LD_LIBRARY_PATH="$(pwd)/../src/share/utf8/.libs:$LD_LIBRARY_PATH"
LD_LIBRARY_PATH=../src/libFLAC/.libs:$LD_LIBRARY_PATH
LD_LIBRARY_PATH=../src/libFLAC++/.libs:$LD_LIBRARY_PATH
LD_LIBRARY_PATH=../src/share/getopt/.libs:$LD_LIBRARY_PATH
LD_LIBRARY_PATH=../src/share/grabbag/.libs:$LD_LIBRARY_PATH
LD_LIBRARY_PATH=../src/share/replaygain_analysis/.libs:$LD_LIBRARY_PATH
LD_LIBRARY_PATH=../src/share/replaygain_synthesis/.libs:$LD_LIBRARY_PATH
LD_LIBRARY_PATH=../src/share/utf8/.libs:$LD_LIBRARY_PATH

export LD_LIBRARY_PATH

PATH="$(pwd)/../objs/$CMAKE_CONFIG_TYPE:$PATH"
PATH="$(pwd)/../objs:$PATH"

EXE=

# Needed for building out-of-tree where source files are in the $top_srcdir tree
# and build products in the $top_builddir tree.
top_srcdir=..
top_builddir=..
git_commit_version_hash=

# Set `is_win` variable which is used in other scripts that source this one.
if test $(env | grep -ic '^comspec=') != 0 ; then
	is_win=yes
else
	is_win=no
fi

# change to 'false' to show all flac/metaflac output (useful for debugging)
if true ; then
	SILENT='--silent'
	TOTALLY_SILENT='--totally-silent'
else
	SILENT=''
	TOTALLY_SILENT=''
fi

# Functions

die ()
{
	echo $* 1>&2
	exit 1
}

make_streams ()
{
	echo "Generating streams..."
	if [ ! -f wacky1.wav ] ; then
		test_streams${EXE} || die "ERROR during test_streams"
	fi
}
