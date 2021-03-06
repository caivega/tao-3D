#!/bin/bash -x
#
# Convenience script to create the automated builds of Tao3D

# Fetch the current release tag (default Jenkins does not do it)
git fetch --tags

# Create a clean build each time we have a "true" release candidate
# Other releases have a git hash beginning with g, like 1.50-1-ge0db3af
REV="$(tao/updaterev.sh -n)"
(echo $REV | grep g) || make distclean

# Configure with the Jenkins input parameters
./configure -$BUILD_OPT $BUILD_VERSION VLC=$VLCPATH 

# Build it and upload on server
make kit
