#!/usr/bin/env bash

STEAMROOT="$(cd "${0%/*}" && echo $PWD)"
STEAMCMD=`basename "$0" .sh`

./builder_linux/steamcmd.sh +login glest_team $1 +run_app_build_http ../scripts/glest_build_linux_578870.vdf +quit
