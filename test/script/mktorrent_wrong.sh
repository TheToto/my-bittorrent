./$1 -m $2/test/torrents/READ 1> empty 2>str
es=$?
res=1
if test $es -eq 1;then
    if ! test "$(cat str)" = ""; then
        if test "$(cat empty)" = ""; then
            if test "$(cat $2/test/torrents/READ.torrent 2> /dev/null)" = ""; then
                res=0
            else
                echo No file should be produced
            fi
        else
            echo "stdout should be empty: $(cat str)"
        fi
    else
        echo Stderr should not be empty:
    fi
else
    echo Exit status: $es
    echo Ref:         1
fi
rm empty 2>/dev/null
rm str 2>/dev/null
rm $2/test/torrents/README.torrent 2>/dev/null
exit $res
