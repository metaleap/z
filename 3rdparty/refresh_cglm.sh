#!/usr/bin/bash
set -e

thisScriptsFilePath="$(readlink --canonicalize-existing "$0")"
thisScriptsDirPath="$(dirname "$thisScriptsFilePath")"
cd $thisScriptsDirPath

depVer="0.9.4"
depDirName="recp_cglm"


### clean up, fetch zip, extract zip:

rm -rf $depDirName
rm -f .tmp.zip
wget -O .tmp.zip https://github.com/recp/cglm/archive/refs/tags/v$depVer.zip
unzip .tmp.zip
rm -f .tmp.zip
mv cglm-$depVer $depDirName
