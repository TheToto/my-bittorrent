$1 -n 1> empty 2>str
es=$?
res=1
if test $es -eq 1;then
    if test "$(cat empty)" = ""; then
        if !test "$(cat str)" = ""; then
            res=0
        else
            echo "An error message must appear on stderr"
        fi
    else
        echo Stdout should be empty: $(cat empty)
    fi
else
    echo Exit status: $es
    echo Ref:         1
fi
rm empty
rm str
exit $res
