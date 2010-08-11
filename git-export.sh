#!/bin/sh

set -e -x

V=$1
N=snapfind-$V

[ x$V != x ]

git archive --format=tar --prefix=$N/ v$V > $N.tar

rm -rf $N
tar xf $N.tar
cd $N
autoreconf -i
rm git-export.sh

cd ..
tar czf $N.tar.gz $N --owner=0 --group=0
rm -r $N $N.tar
