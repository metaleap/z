#!/usr/bin/bash
set -e

thisScriptsFilePath="$(readlink --canonicalize-existing "$0")"
thisScriptsDirPath="$(dirname "$thisScriptsFilePath")"
cd $thisScriptsDirPath

depVer="1.91.7" # NOTE! keep `imgui` version in lockstep with sibling `cimgui`
depDirName="ocornut_imgui"


### clean up, fetch zip, extract zip:

rm -rf $depDirName
rm -f .tmp.zip
wget -O .tmp.zip https://github.com/ocornut/imgui/archive/refs/tags/v$depVer.zip
unzip .tmp.zip
rm -f .tmp.zip
mv imgui-$depVer $depDirName
