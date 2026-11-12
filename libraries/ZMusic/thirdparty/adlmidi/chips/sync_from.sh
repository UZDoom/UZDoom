#!/bin/bash

if [[ "$OSTYPE" == "darwin"* ]]; then
    RSYNC_ARGS=-vrt
    SRC=$PWD/../../../OPL_ChipSet
    DST=$PWD
else
    RSYNC_ARGS=-vArXtU
    SRC=`realpath $PWD/../../../OPL_ChipSet`
    DST=$PWD
fi

if [[ ! -d $SRC ]]; then
    echo "Can't sync! You should clone this repository https://github.com/Wohlstand/OPL_ChipSet and place it in the same directory as libADLMIDI's one. It should be named as \"OPL_ChipSet\"."
    exit 1
fi

cd "$SRC"
git pull

cd "$DST"

rsync $RSYNC_ARGS --exclude ".git" --exclude ".gitattributes" "$SRC/"* "$DST/"
