#!/bin/bash
if [ $# -lt 1 ]; then
    echo $0 folder
    exit
fi

rm score.txt
n=0
maxjobs=8
for file in $1/*.jpg
do
    case "$file" in
        *profile.jpg ) 
            echo "skipped" $file;;
        * ) 
            echo $file
            ./matting -e -g -m $file >> score.txt &
            if (( $(($((++n)) % $maxjobs)) == 0 )) ; then
                wait # wait until all have finished (not optimal, but most times good enough)
                echo $n wait
            fi
            ;;
    esac
    echo done
done
