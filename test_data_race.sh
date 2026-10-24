#!/bin/bash
# Test de data race pour Codexion (a lancer depuis la racine du depot)
# Usage : bash test_data_race.sh
# Outils : valgrind (helgrind) et gcc -fsanitize=thread (ThreadSanitizer)

SRC_DIR="coders"
TSAN_BIN="/tmp/codexion_tsan"
GREEN="\033[32m"; RED="\033[31m"; END="\033[0m"
FAIL=0

# Chaque ligne = une configuration a tester
CONFIGS=(
	"5 800 100 100 100 3 20 fifo"
	"5 800 100 100 100 3 20 edf"
	"30 3000 50 50 50 3 10 edf"
	"4 410 200 200 200 3 0 edf"
	"1 300 100 100 100 2 10 fifo"
	"3 100 20 20 100 5 10 edf"
)

report()
{
	if [ "$2" -eq 0 ]; then
		echo -e "  ${GREEN}[PASS]${END} $1"
	else
		echo -e "  ${RED}[FAIL]${END} $1 ($2 probleme(s))"
		FAIL=1
	fi
}

echo "=== 1) Helgrind (valgrind) ==="
if ! command -v valgrind > /dev/null; then
	echo "  valgrind absent : etape ignoree"
else
	make -s > /dev/null 2>&1 || { echo "make a echoue"; exit 1; }
	for cfg in "${CONFIGS[@]}"; do
		valgrind --tool=helgrind -q ./codexion $cfg > /dev/null 2> /tmp/hg.txt
		n=$(grep -cE "Possible data race|lock order|Misuse of POSIX" /tmp/hg.txt)
		report "helgrind : $cfg" "$n"
	done
fi

echo "=== 2) ThreadSanitizer (gcc) ==="
if ! cc -g -O0 -fsanitize=thread -pthread -I "$SRC_DIR" "$SRC_DIR"/*.c \
	-o "$TSAN_BIN" 2> /dev/null; then
	echo "  compilation TSan impossible : etape ignoree"
else
	for cfg in "${CONFIGS[@]}"; do
		"$TSAN_BIN" $cfg > /dev/null 2> /tmp/tsan.txt
		n=$(grep -c "WARNING: ThreadSanitizer" /tmp/tsan.txt)
		report "tsan     : $cfg" "$n"
	done
fi

echo
if [ "$FAIL" -eq 0 ]; then
	echo -e "${GREEN}Aucune data race detectee.${END}"
else
	echo -e "${RED}Des problemes ont ete detectes : voir /tmp/hg.txt et /tmp/tsan.txt${END}"
fi
exit $FAIL
