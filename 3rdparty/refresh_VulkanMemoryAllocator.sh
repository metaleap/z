#!/usr/bin/bash
set -e

thisScriptsFilePath="$(readlink --canonicalize-existing "$0")"
thisScriptsDirPath="$(dirname "$thisScriptsFilePath")"
cd $thisScriptsDirPath

depVer="3.2.1"
depDirName="GPUOpen-LibrariesAndSDKs_VulkanMemoryAllocator"


### clean up, fetch zip, extract zip:

rm -rf $depDirName
rm -f .tmp.zip
wget -O .tmp.zip https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/archive/refs/tags/v$depVer.zip
unzip .tmp.zip
rm -f .tmp.zip
mv VulkanMemoryAllocator-$depVer $depDirName
