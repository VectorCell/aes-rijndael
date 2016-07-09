#!/bin/bash

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
dir=$(dirname $(readlink -f $DIR/$(basename "$0")))

if [ "$(whoami)" == root ]; then
	bindir="/usr/local/bin"
else
	bindir="$HOME/bin"
fi
cd $bindir

ln -s $DIR/aes
