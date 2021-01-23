#!/bin/bash

cur_dir=$(basename $(
    cd $(dirname ${BASH_SOURCE[0]})
    pwd
))

if [ $cur_dir != "script" ]; then

    source ../start_base.sh

    echo "Collecting Trace Files..."
    /abacus/Intel-Pin-Archive/pin -t /abacus/Pintools/CryptoLibrary/obj-ia32/$alg_str-$lib_str-$ver_str.so -- $bin_str

    echo "Analyzing"
    /abacus/QIF-new /abacus/script/$dir_str/Inst_data.txt -o ${dir_str,,}.txt

else
    for i in $(find -type d); do
        if [ ! $i = "." ]; then
            # if [ ! -x $i/start.sh ]; then
            cp /abacus/script/start.sh $i/
            # fi
            cd $i
            echo $i
            source start.sh
            cd ..
        fi
    done
fi
