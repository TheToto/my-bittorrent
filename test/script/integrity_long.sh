./$1 "--check-integrity" "$2/test/torrents/Congratulations.txt_basic.torrent" 1> empty 2>str
es=$?
res=1
if test $es -eq 1;then
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
    echo Ref:         1
fi
rm empty 2>/dev/null
rm str 2>/dev/null
exit $res
