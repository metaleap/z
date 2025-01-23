#!/usr/bin/bash
set -e

thisScriptsFilePath="$(readlink --canonicalize-existing "$0")"
thisScriptsDirPath="$(dirname "$thisScriptsFilePath")"
cd $thisScriptsDirPath



rm -rf .cache
rm -rf .build
mkdir .build

mkdir .build/debug
meson setup --native-file meson_native.ini .build/debug

mkdir .build/release_gcc
meson setup -Dbuildtype=release -Dprefer_static=true .build/release_gcc

mkdir .build/release_clang
CC=clang CXX=clang++ meson setup -Dbuildtype=release -Dprefer_static=true .build/release_clang

cp .build/debug/compile_commands.json .
