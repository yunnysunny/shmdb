#!/bin/sh
# This shell script runs Autotools to generate the build environment

# creates the local environment, sets up m4 macros
aclocal

# enables libtool in automake
libtoolize --automake

# creates some files that --add-missing can't, but are required to create
# the Makefile.in
#touch NEWS README AUTHORS ChangeLog

# turns Makefile.am into Makefile.in
automake --add-missing

# turns configure.ac into the pure shell configure
autoconf
