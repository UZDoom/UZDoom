#!/bin/bash

DST=`realpath $PWD/../../../OPL_ChipSet`
SRC=$PWD

if [[ ! -d $SRC ]]; then
    echo "Can't sync! You should clone this repository https://github.com/Wohlstand/OPL_ChipSet and place it in the same directory as libADLMIDI's one. It should be named as \"OPL_ChipSet\"."
    exit 1
fi

rsync -vArXtU --exclude "sync_from.sh" --exclude "sync_to.sh" "$SRC/"* "$DST/"
