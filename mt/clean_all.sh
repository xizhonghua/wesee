for file in data/train-profiles/*.jpg
do
    case "$file" in
        *profile.jpg ) 
            echo "skipped" $file;;
        * )
          pro=`echo $file | sed -e "s/.jpg/-profile.jpg/"`
          echo $pro
          if [ ! -f $pro ]; then
            rm $file
            echo $file has no profile removed
          fi
    esac
done
