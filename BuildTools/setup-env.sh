#!/bin/bash -e

echo "Setup environment"

[ "$FO_ROOT" ] || export FO_ROOT="$(cd $(dirname ${BASH_SOURCE[0]})/../ && pwd)"
[ "$FO_WORKSPACE" ] || export FO_WORKSPACE=$PWD/Workspace

export FO_ROOT=$(cd $FO_ROOT; pwd)
export FO_WORKSPACE=$(mkdir -p $FO_WORKSPACE; cd $FO_WORKSPACE; pwd)
export EMSCRIPTEN_VERSION="1.39.8"
export ANDROID_HOME="/usr/lib/android-sdk"
export ANDROID_NDK_VERSION="android-ndk-r18b"
export ANDROID_SDK_VERSION="tools_r25.2.3"
export ANDROID_NATIVE_API_LEVEL_NUMBER=21

echo "- FO_ROOT=$FO_ROOT"
echo "- FO_WORKSPACE=$FO_WORKSPACE"
echo "- FO_CMAKE_CONTRIBUTION=$FO_CMAKE_CONTRIBUTION"
echo "- EMSCRIPTEN_VERSION=$EMSCRIPTEN_VERSION"
echo "- ANDROID_HOME=$ANDROID_HOME"
echo "- ANDROID_NDK_VERSION=$ANDROID_NDK_VERSION"
echo "- ANDROID_SDK_VERSION=$ANDROID_SDK_VERSION"
echo "- ANDROID_NATIVE_API_LEVEL_NUMBER=$ANDROID_NATIVE_API_LEVEL_NUMBER"
echo "- SCE_ORBIS_SDK_DIR=$SCE_ORBIS_SDK_DIR"

cd $FO_WORKSPACE
