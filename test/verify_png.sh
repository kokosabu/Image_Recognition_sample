#!/bin/sh

# $#

files="./PngSuite-2017jul19/*.png"
for filepath in $files; do

    ORIGINAL_FILE=$filepath
    FILE1=${1/.png/_1.bmp}
    FILE2=${1/.png/2.bmp}
    FILE3=${1/.png/diff.bmp}

    echo $ORIGINAL_FILE
    # ループ
    ../src/my_image_decoder $ORIGINAL_FILE $FILE1 &> /dev/null
    if [ -e $FILE1 ]; then
        # 成功なら
        convert $ORIGINAL_FILE $FILE2 &> /dev/null
        composite -compose difference $FILE1 $FILE2 $FILE3 &> /dev/null
        RET=`identify -format "%[mean]" $FILE3`
        if [ $RET -eq 0 ]; then
            echo "一致"
        else
            echo "不一致"
        fi
    fi
    # -> 0なら一致

done
