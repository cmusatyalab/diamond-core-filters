#!/bin/sh

RELEASE_NAME=snapfind_satya_v2

#
# Make sure we have cleaned the source tree
#
pushd ../src
export SNAPFIND_ROOT=`pwd`
make clean
popd

#
# Remove the old version in /tmp if it exists
#
rm -rf /tmp/"$RELEASE_NAME"

#
# copy over the whole tree
#
cp -r ../src /tmp/"$RELEASE_NAME"/


# generate the tar file

pushd /tmp
tar -czf /tmp/"$RELEASE_NAME".tgz "$RELEASE_NAME"

popd
cp /tmp/"$RELEASE_NAME".tgz .
