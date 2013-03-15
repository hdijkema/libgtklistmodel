#/bin/bash
VER=`grep AC_INIT ../configure.ac | sed -e 's/AC_INIT(//' | sed -e 's/)//' | sed -e 's/[^,]*[,]//'`
echo $VER
