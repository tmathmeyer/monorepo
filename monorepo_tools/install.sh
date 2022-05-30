#!/bin/sh

pushd $(dirname "$0")
cd ..
git clone git@github.com:tmathmeyer/impulse.git
cd impulse
make
cd ..
ln -s impulse/rules rules
./impulse/GENERATED/BINARIES/impulse/impulse init
./impulse/GENERATED/BINARIES/impulse/impulse build //impulse:impulse --debug --force
./GENERATED/BINARIES/impulse/impulse build //monorepo_tools:mono --debug --force
./GENERATED/BINARIES/monorepo_tools/mono sync
cd impulse
make clean


popd
