#!/usr/bin/bash
set -e

thisScriptsFilePath="$(readlink --canonicalize-existing "$0")"
thisScriptsDirPath="$(dirname "$thisScriptsFilePath")"
cd $thisScriptsDirPath

depVer="1.14"
depDirName="jkuhlmann_cgltf"


### clean up, fetch zip, extract zip:

rm -rf $depDirName
rm -f .tmp.zip
wget -O .tmp.zip https://github.com/jkuhlmann/cgltf/archive/refs/tags/v$depVer.zip
unzip .tmp.zip
rm -f .tmp.zip
mv cgltf-$depVer $depDirName
