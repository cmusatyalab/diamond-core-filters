#!/bin/sh

RELEASE_NAME=snapfind-0.8.0
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

# remove the CVS directories
find /tmp/"$RELEASE_NAME" -name CVS -exec rm -rf {} \;

# generate the tar file

pushd /tmp
tar -cf /tmp/"$RELEASE_NAME".tar "$RELEASE_NAME"

popd
cp /tmp/"$RELEASE_NAME".tar .

gzip "$RELEASE_NAME".tar 
