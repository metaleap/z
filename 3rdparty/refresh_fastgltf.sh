#!/usr/bin/bash
set -e

thisScriptsFilePath="$(readlink --canonicalize-existing "$0")"
thisScriptsDirPath="$(dirname "$thisScriptsFilePath")"
cd $thisScriptsDirPath

depVer="0.8.0"
depDirName="spnda_fastgltf"


### clean up, fetch zip, extract zip:

rm -rf $depDirName
rm -f .tmp.zip
wget -O .tmp.zip https://github.com/spnda/fastgltf/archive/refs/tags/v$depVer.zip
unzip .tmp.zip
rm -f .tmp.zip
mv fastgltf-$depVer $depDirName


### build:

cd $depDirName
rm -rf .build
mkdir .build
cmake -S . -B .build
cd .build && make && cd ..
cd ..
