$1 "-p" "$2/test/torrents/README" 1> empty 2>str
es=$?
res=1
if test $es -eq 1;then
    if test "$(cat empty)" = ""; then
        if ! test "$(cat str)" = ""; then
            res=0
        else
            echo "An error message must appear"
        fi
    else
        echo As an error occured, no output should be printed
        cat empty
    fi
else
    echo Exit status: $es
    echo Ref:         1
fi
rm empty
rm str
exit $res
