# Author: welbon
# Date: 2011-12-26
# Mail: libo.weng@gmail.com

aclocal

glibtoolize -f -c

autoconf

automake --add-missing

./configure --prefix=$(pwd)/..

make clean

make install
