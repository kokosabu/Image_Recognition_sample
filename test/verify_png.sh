#!/bin/sh

files="./PngSuite-2017jul19/*.png"
for filepath in $files; do

    ORIGINAL_FILE=$filepath
    MY_DECODE_FILE=${filepath/.png/_1.bmp}
    IMAGEMAGICK_FILE=${filepath/.png/_2.bmp}
    DIFF_FILE=${filepath/.png/_diff.bmp}

    echo $ORIGINAL_FILE
    ../src/my_image_decoder $ORIGINAL_FILE $MY_DECODE_FILE &> /dev/null
    if [ -e $MY_DECODE_FILE ]; then
        convert $ORIGINAL_FILE bmp3:$IMAGEMAGICK_FILE &> /dev/null
        composite -compose difference $MY_DECODE_FILE $IMAGEMAGICK_FILE $DIFF_FILE &> /dev/null
        RET=`identify -format "%[mean]" $DIFF_FILE`
        if [ $RET -eq 0 ]; then
            echo "一致"
        else
            echo "不一致"
        fi
    else
        echo "デコード失敗"
    fi

done
