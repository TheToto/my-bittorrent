./$1 "-m" "$2/test/torrents/" 1> empty 2>str
./$1 "-c" "$2/test/torrents/.torrent" 1> empty 2>str
es=$?
res=1
if test $es -eq 0;then
    if test "$(cat str)" = ""; then
        if test "$(cat empty)" = ""; then
            res=0
        else
            echo Output: $(cat str)
            echo "stdout should be empty"
        fi
    else
        echo Stderr should be empty: $(cat str)
    fi
else
    echo Exit status: $es
    echo Ref:         0
fi
rm empty 2>/dev/null
rm str 2>/dev/null
rm $2/test/torrents/.torrent 2> /dev/null
exit $res
