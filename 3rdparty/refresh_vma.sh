#!/usr/bin/bash
set -e

thisScriptsFilePath="$(readlink --canonicalize-existing "$0")"
thisScriptsDirPath="$(dirname "$thisScriptsFilePath")"
cd $thisScriptsDirPath

depVer="3.2.0"


### clean up, fetch zip, extract zip:

rm -rf GPUOpen-LibrariesAndSDKs___VulkanMemoryAllocator
rm -f vma.zip
wget -O vma.zip https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/archive/refs/tags/v$depVer.zip
unzip vma.zip
rm -f vma.zip
mv VulkanMemoryAllocator-$depVer GPUOpen-LibrariesAndSDKs___VulkanMemoryAllocator



### build:

cd GPUOpen-LibrariesAndSDKs___VulkanMemoryAllocator
cmake -S . -B .build
cmake --install .build --prefix .build/install
cd ..
