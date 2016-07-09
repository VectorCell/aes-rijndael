#!/bin/bash


echo -n "Running acceptance tests ... "


echo "Hello World!" | md5sum > original.md5
echo "Hello World!" | ./aes e | ./aes d | md5sum > verify.md5
if [ -n "$(diff original.md5 verify.md5)" ]; then
	echo "FAIL"
	exit 1
fi

echo "abcdefghijklmno" | md5sum > original.md5
echo "abcdefghijklmno" | ./aes e | ./aes d | md5sum > verify.md5
if [ -n "$(diff original.md5 verify.md5)" ]; then
	echo "FAIL"
	exit 1
fi

echo "Hello world, this is Brandon! I'm so pleased to meet you!" | md5sum > original.md5
echo "Hello world, this is Brandon! I'm so pleased to meet you!" | ./aes e | ./aes d | md5sum > verify.md5
if [ -n "$(diff original.md5 verify.md5)" ]; then
	echo "FAIL"
	exit 1
fi

cat aes.cc | md5sum > original.md5
cat aes.cc | ./aes e | ./aes d | md5sum > verify.md5
if [ -n "$(diff original.md5 verify.md5)" ]; then
	echo "FAIL"
	exit 1
fi

cat aes.cc | md5sum > original.md5
cat aes.cc | ./aes e -s 256 | ./aes d -s 256 | md5sum > verify.md5
if [ -n "$(diff original.md5 verify.md5)" ]; then
	echo "FAIL"
	exit 1
fi

cat aes.cc | md5sum > original.md5
cat aes.cc | ./aes e -m cbc | ./aes d -m cbc | md5sum > verify.md5
if [ -n "$(diff original.md5 verify.md5)" ]; then
	echo "FAIL"
	exit 1
fi

cat aes.cc | md5sum > original.md5
cat aes.cc | ./aes e -m cbc -s 256 | ./aes d -m cbc -s 256 | md5sum > verify.md5
if [ -n "$(diff original.md5 verify.md5)" ]; then
	echo "FAIL"
	exit 1
fi

echo "PASS"

rm original.md5 verify.md5
