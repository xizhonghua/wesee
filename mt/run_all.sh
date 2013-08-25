rm score.txt
for file in data/train-profiles/*.jpg
do
    case "$file" in
        *profile.jpg ) 
            echo "skipped" $file;;
        * ) 
            echo $file
            ./matting -e -g -m $file >> score.txt;;
    esac
done
