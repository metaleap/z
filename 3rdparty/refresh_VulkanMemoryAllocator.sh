#!/usr/bin/bash
set -e

thisScriptsFilePath="$(readlink --canonicalize-existing "$0")"
thisScriptsDirPath="$(dirname "$thisScriptsFilePath")"
cd $thisScriptsDirPath

depVer="3.2.0"
depDirName="GPUOpen-LibrariesAndSDKs___VulkanMemoryAllocator"


### clean up, fetch zip, extract zip:

rm -rf $depDirName
rm -f VulkanMemoryAllocator.zip
wget -O VulkanMemoryAllocator.zip https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/archive/refs/tags/v$depVer.zip
unzip VulkanMemoryAllocator.zip
rm -f VulkanMemoryAllocator.zip
mv VulkanMemoryAllocator-$depVer $depDirName



### build:

cd $depDirName
cmake -S . -B .build
cmake --install .build --prefix .build/install
cd ..
