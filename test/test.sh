cur=$(dirname $0);
cur="$cur"/../
echo $cur
echo -en "\033[5m"
echo -E "___  ___       ______ _ _ _____                         _"
echo -E "|  \/  |       | ___ (_) |_   _|                       | |"
echo -E "| .  . |_   _  | |_/ /_| |_| | ___  _ __ _ __ ___ _ __ | |_"
echo -E "| |\/| | | | | | ___ \ | __| |/ _ \| '__| '__/ _ \ '_ \| __|"
echo -E "| |  | | |_| | | |_/ / | |_| | (_) | |  | | |  __/ | | | |_"
echo -E "\_|  |_/\__, | \____/|_|\__\_/\___/|_|  |_|  \___|_| |_|\__|"
echo -E "         __/ |"
echo -E "        |___/"
echo -e "\033[25m"

tmp1=$(ls "$cur"/test/script)

cd "$cur"
mkdir build 2>/dev/null
cd build
cmake .. > /dev/null
echo
echo -en "\033[33mBuilding"
echo -e "\033[033m...\033[33m"
make
if test $? -eq 1;then
    echo -e "\033[31mFAILED\033[33m"
    exit 1
fi

echo COMPLETE
sleep 1
clear

echo "-----------"
echo "-  TESTS  -"
echo "-----------"
echo

success=0
failed=0

for file in $tmp1; do
    ./"$cur"/test/script/$file "$cur"/build/my-bittorrent > tmp
    if test $? -eq 0; then
        echo -e "\033[32mPASSED\033[33m:  test/script/$file"
        cat tmp
        success=$((success + 1))
    else
        echo -e "\e[31mFAILED\033[33m:  test/script/$file"
        cat tmp
        failed=$((failed + 1))
    fi
    echo
done

rm tmp

echo
echo "-----------"
echo "- RESULTS -"
echo "-----------"
echo

echo -e "\033[32mPASSED\033[33m:  $success"
echo -e "\033[31mFAILED\033[33m:  $failed"
echo
echo "TOTAL:   $((success + failed))"
echo -e "\033[0m"
