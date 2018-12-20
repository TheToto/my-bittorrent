cur=$(dirname $0);
cur="$cur"/../
echo $cur
echo -en "\e[5m"
echo -E "___  ___       ______ _ _ _____                         _"
echo -E "|  \/  |       | ___ (_) |_   _|                       | |"
echo -E "| .  . |_   _  | |_/ /_| |_| | ___  _ __ _ __ ___ _ __ | |_"
echo -E "| |\/| | | | | | ___ \ | __| |/ _ \| '__| '__/ _ \ '_ \| __|"
echo -E "| |  | | |_| | | |_/ / | |_| | (_) | |  | | |  __/ | | | |_"
echo -E "\_|  |_/\__, | \____/|_|\__\_/\___/|_|  |_|  \___|_| |_|\__|"
echo -E "         __/ |"
echo -E "        |___/"
echo -e "\e[25m"

tmp1=$(ls "$cur"/test/script)

cd "$cur"
mkdir build 2>/dev/null
cd build
cmake .. > /dev/null
echo
echo -en "\e[25mBuilding"
echo -e "\e[5m...\e[25m"
make
if test $? -eq 1;then
    echo -e "\e[91mFAILED\e[39m"
    exit 1
fi

echo COMPLETE
echo TESTS
echo

success=0
failed=0

for file in $tmp1; do
    ./"$cur"/test/script/$file "$cur"/build/my-bittorrent &> /dev/null
    if test $? -eq 0; then
        echo -en "\e[92mPASSED: \e[25m $cur/test/scipt/$file"
        success=$((success++))
    else
        echo -en "\e[91mFAILED: \e[25m $cur/test/scipt/$file"
        failed=$((failed++))
    fi
done

echo
echo "-----------"
echo "- RESULTS -"
echo "-----------"
echo

echo -e "\e[92mPASSED: \e[0m $success"
echo -e "\e[91mFAILED: \e[0m $failed"
