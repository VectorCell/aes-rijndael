#!/bin/bash

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
dir=$(dirname $(readlink -f $DIR/$(basename "$0")))

if [ "$(whoami)" == root ]; then
	bindir="/usr/local/bin"
else
	bindir="$HOME/bin"
fi
cd $bindir

find $dir -maxdepth 1 -perm -111 -type f | grep -v "\.sh$" | while read file; do
	installed="$bindir/$(basename $file)"
	if [ -e $installed ]; then
		echo rm $installed
		rm $installed
	fi
done
