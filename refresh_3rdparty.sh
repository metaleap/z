#!/usr/bin/bash
set -e

thisScriptsFilePath="$(readlink --canonicalize-existing "$0")"
thisScriptsDirPath="$(dirname "$thisScriptsFilePath")"
cd $thisScriptsDirPath

cd 3rdparty
./refresh_VulkanMemoryAllocator.sh
./refresh_imgui.sh
./refresh_cglm.sh
cd ..
