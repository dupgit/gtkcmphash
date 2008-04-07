#!/bin/bash
#
# Author : Olivier Delhomme
# Licence : GNU GPL v2
#
if [ -f Makefile ];
 then
    make distclean
fi;
rm -f aclocal.m4
echo "rm -f aclocal.m4"
rm -fr autom4te.cache
echo "rm -fr autom4te.cache"
rm -f config.guess
echo "rm -f config.guess"
rm -f config.sub
echo "rm -f config.sub"
rm -f depcomp
echo "rm -f depcomp"
rm -f INSTALL
echo "rm -f INSTALL"
rm -f install-sh
echo "rm -f install-sh"
rm -f ltmain.sh
echo "rm -f ltmain.sh"
rm -f missing
echo "rm -f missing"
rm -f src/*~ src/*#
echo "rm -f src/*~ src/*#"
rm -f *~ *#
echo "rm -f *~ *#"
rm -f Makefile.in src/Makefile.in
echo "rm -f Makefile.in src/Makefile.in"
rm -f configure
echo "rm -f configure"
rm -f config.status config.log
echo "rm -f config.status config.log"
rm -f libtool
echo "rm -f libtool"
find -name *~ -exec rm -f {} \;
echo "find -name *~ -exec rm {} \;"
find -name *.gcd[ao] -exec rm -f {} \;
echo "find -name *.gcd[ao] -exec rm -f {} \;"
find -name *.gcov -exec rm -f {} \;
echo "find -name *.gcov -exec rm -f {} \;"
find -name *.gcno -exec rm -f {} \;
echo "find -name *.gcno -exec rm -f {} \;"
rm -f intltool-extract.in
echo "rm -f intltool-extract.in"
rm -f intltool-merge.in
echo "rm -f intltool-merge.in"
rm -f intltool-update.in
echo "rm -f intltool-update.in"
rm -f intltool-extract
echo "rm -f intltool-extract"
rm -f intltool-merge
echo "rm -f intltool-merge"
rm -f intltool-update
echo "rm -f intltool-update"
rm -f src/gmon.out
echo "rm -f src/gmon.out"


