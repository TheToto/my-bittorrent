rm "$(pwd)/Congratulations.txt"
./$1 -v $2/test/torrents/Congratulations.txt_basic.torrent 1>out 2>err
es=$?
if test $es -ne 0; then
    echo "Exit status: $?"
    echo "Ref:         0"
    exit 1
fi
nb=$(cat out | wc -l)

if test $nb -lt 5; then
    echo Error: Log did not match
    cat out
    exit 1
fi

if ! test "$(cat err)" = ""; then
    echo Error message was not expected
    cat err
    exit 1
fi

./$1 -c $2/test/torrents/Congratulations.txt_basic.torrent 1>out 2>err
es=$?

if test $es -ne 0; then
    echo "Integrity check failed"
    exit 1
fi

pt=$(pwd)
rm "$pt/out"
rm "$pt/Congratulations.txt"
rm "$pt/err"

exit 0
