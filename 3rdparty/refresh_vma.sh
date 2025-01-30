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



### build WickedEngine twice:

# cd .wi

# rm -rf .build_RelWithDebInfo
# mkdir .build_RelWithDebInfo
# cd .build_RelWithDebInfo
# cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -DWICKED_PIC=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
# make
# cp compile_commands.json ../compile_commands.json
# cd ..

# rm -rf .build_Release
# mkdir .build_Release
# cd .build_Release
# cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
# make
# cd ..

# cd ..
