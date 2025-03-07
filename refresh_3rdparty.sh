#!/usr/bin/bash
set -e

thisScriptsFilePath="$(readlink --canonicalize-existing "$0")"
thisScriptsDirPath="$(dirname "$thisScriptsFilePath")"
cd $thisScriptsDirPath

cd 3rdparty
./refresh_VulkanMemoryAllocator.sh
./refresh_imgui.sh # this must come before cimgui
./refresh_cimgui.sh
./refresh_cglm.sh
./refresh_cgltf.sh
./refresh_fastgltf.sh
./refresh_GenericDataStructures.sh
cd ..
