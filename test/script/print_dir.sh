$1 "-m" "$2/test/torrents" &> /dev/null
$1 "-p" "torrents.torrent" 1> empty 2>str
es=$?
res=1
if test $es -eq 0;then
    if ! test "$(cat empty)" = ""; then
        if test "$(cat str)" = ""; then
            res=0
        else
            echo "No error message must appear"
            cat str
        fi
    else
        echo A JSON output must be printed on stdout
    fi
else
    echo Exit status: $es
    echo Ref:         0
fi
rm empty
rm str
rm "torrents.torrent"
exit $res
