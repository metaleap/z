#!/usr/bin/bash
set -e

thisScriptsFilePath="$(readlink --canonicalize-existing "$0")"
thisScriptsDirPath="$(dirname "$thisScriptsFilePath")"
cd $thisScriptsDirPath

depVer="1.91.8" # NOTE!! keep in sync with sibling dep `cimgui`
depDirName="ocornut_imgui"


### clean up, fetch zip, extract zip:

rm -rf $depDirName
rm -f .tmp.zip
wget -O .tmp.zip https://github.com/ocornut/imgui/archive/refs/tags/v$depVer.zip
unzip .tmp.zip
rm -f .tmp.zip
mv imgui-$depVer $depDirName
