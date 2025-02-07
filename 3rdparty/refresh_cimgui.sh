#!/usr/bin/bash
set -e

thisScriptsFilePath="$(readlink --canonicalize-existing "$0")"
thisScriptsDirPath="$(dirname "$thisScriptsFilePath")"
cd $thisScriptsDirPath

depVer="1.91.8" # NOTE!! keep in sync with sibling dep `imgui`
depDirName="cimgui_cimgui"


### clean up, fetch zip, extract zip:

rm -rf $depDirName
rm -f .tmp.zip
wget -O .tmp.zip https://github.com/cimgui/cimgui/archive/refs/tags/$depVer.zip
unzip .tmp.zip
rm -f .tmp.zip
mv cimgui-$depVer $depDirName
cp -rf ocornut_imgui/* $depDirName/imgui/
