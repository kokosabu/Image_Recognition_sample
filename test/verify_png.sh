#!/bin/sh

# $#

# ループ
../src/my_image_decoder $1 &> /dev/null
if [ -e ./test.bmp ]; then
    # 成功なら
    convert $1 test2.bmp &> /dev/null
    composite -compose difference test.bmp test2.bmp diff.bmp &> /dev/null
    VAR1=`identify -format "%[mean]" diff.bmp`
    if [ $VAR1 -eq 0 ]; then
        echo "一致"
    else
        echo "不一致"
    fi
fi
# -> 0なら一致
