#!/bin/sh

# $#

FILE=${1/png/bmp}
FILE2=${1/.png/2.bmp}

# ループ
../src/my_image_decoder $1 $FILE &> /dev/null
if [ -e $FILE ]; then
    # 成功なら
    convert $1 $FILE2 &> /dev/null
    composite -compose difference $FILE $FILE2 diff.bmp &> /dev/null
    RET=`identify -format "%[mean]" diff.bmp`
    if [ $RET -eq 0 ]; then
        echo "一致"
    else
        echo "不一致"
    fi
fi
# -> 0なら一致
