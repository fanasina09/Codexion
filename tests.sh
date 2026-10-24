#!/bin/bash
# ****************************************************************** #
#  tests.sh - Script de tests automatiques pour le projet Codexion.  #
#  A placer a cote du Makefile (les sources peuvent etre dans src/). #
#                                                                    #
#      chmod +x tests.sh                                             #
#      ./tests.sh                                                    #
# ****************************************************************** #

# Se placer dans le dossier du script (peu importe d'ou on le lance)
cd "$(dirname "$0")" || exit 1

# Locale UTF-8 : sans elle, ${#s} compte des octets et les cadres
# (emojis, ═, ║) sont decales.
for loc in C.UTF-8 en_US.UTF-8 fr_FR.UTF-8; do
	if locale -a 2> /dev/null | grep -qix "${loc//-/}\|$loc"; then
		export LC_ALL="$loc"
		break
	fi
done

BIN="./codexion"
PASS=0
FAIL=0
FAILED_TESTS=()

GREEN="\033[0;32m"
RED="\033[0;31m"
YELLOW="\033[0;33m"
CYAN="\033[0;36m"
MAGENTA="\033[0;35m"
BOLD="\033[1m"
NC="\033[0m"

ok() {
	PASS=$((PASS + 1))
	echo -e "${GREEN}[PASS]${NC} $1"
}

ko() {
	FAIL=$((FAIL + 1))
	FAILED_TESTS+=("$1")
	echo -e "${RED}[FAIL]${NC} $1"
}

info() {
	echo -e "${YELLOW}--- $1 ---${NC}"
}

EMOJI_REGEX='💻|🚀|🎉|😂|🤣|🔥|😭|💀|🙏|😢'

visual_width() {
	local s="$1"
	local charlen=${#s}
	local ecount
	ecount=$(printf '%s' "$s" | grep -oE "$EMOJI_REGEX" 2> /dev/null | wc -l)
	echo $((charlen + ecount))
}

repeat_char() {
	local char="$1"
	local n="$2"
	local i=0
	while [ "$i" -lt "$n" ]; do
		printf '%s' "$char"
		i=$((i + 1))
	done
}

print_box() {
	local color="$1"
	shift
	local lines=("$@")
	local maxlen=0
	local l len
	for l in "${lines[@]}"; do
		len=$(visual_width "$l")
		[ "$len" -gt "$maxlen" ] && maxlen=$len
	done
	local width=$((maxlen + 4))
	echo -e "${color}${BOLD}"
	printf '╔'; repeat_char '═' "$width"; printf '╗\n'
	for l in "${lines[@]}"; do
		len=$(visual_width "$l")
		local pad_left=$(( (width - len) / 2 ))
		local pad_right=$(( width - len - pad_left ))
		printf '║'; repeat_char ' ' "$pad_left"; printf '%s' "$l"
		repeat_char ' ' "$pad_right"; printf '║\n'
	done
	printf '╚'; repeat_char '═' "$width"; printf '╝\n'
	echo -e "${NC}"
}

print_box "$CYAN" \
	"💻  CODEXION - TEST SUITE  💻" \
	"" \
	"🚀 On lance les tests 🚀"

# ---------------------------------------------------------------- #
# 0. Compilation
# ---------------------------------------------------------------- #

info "Compilation"
if make re > /tmp/codexion_build.log 2>&1; then
	ok "Compilation sans erreur (make re)"
else
	ko "Compilation (voir /tmp/codexion_build.log)"
	echo "Impossible de continuer sans binaire compile."
	exit 1
fi

if grep -qi "warning" /tmp/codexion_build.log; then
	ko "Compilation sans aucun warning (-Wall -Wextra -Werror)"
	grep -i "warning" /tmp/codexion_build.log
else
	ok "Compilation sans aucun warning"
fi

if [ ! -x "$BIN" ]; then
	echo "Le binaire $BIN n'existe pas ou n'est pas executable."
	exit 1
fi

# ---------------------------------------------------------------- #
# 1. Arguments invalides
#    Attendu : retour != 0, pas de crash, pas de timeout.
# ---------------------------------------------------------------- #

info "1. Arguments invalides"

check_invalid() {
	local desc="$1"
	shift
	local out
	out=$(timeout 2 "$BIN" "$@" 2>&1)
	local ret=$?
	if [ $ret -eq 124 ]; then
		ko "$desc (timeout : la simulation a ete lancee au lieu de rejeter)"
	elif [ $ret -ge 128 ]; then
		ko "$desc (crash detecte, code=$ret)"
	elif [ $ret -ne 0 ]; then
		ok "$desc (retour=$ret, rejete correctement)"
	else
		ko "$desc (retour=0, devrait etre rejete. Sortie: $out)"
	fi
}

# Aucun argument : on accepte soit "return 0 sans sortie", soit une
# erreur propre (retour != 0). Seuls un crash ou un timeout echouent.
out=$(timeout 1 "$BIN" 2>&1)
ret=$?
if [ $ret -eq 124 ] || [ $ret -ge 128 ]; then
	ko "Aucun argument (timeout ou crash, retour=$ret)"
else
	ok "Aucun argument -> sortie propre (retour=$ret)"
fi

check_invalid "7 arguments au lieu de 8"        4 800 200 200 200 5 10
check_invalid "9 arguments (trop)"              4 800 200 200 200 5 10 fifo extra
check_invalid "nb_coders negatif"               -4 800 200 200 200 5 10 fifo
check_invalid "nb_coders = 0"                   0 800 200 200 200 5 10 fifo
check_invalid "burnout_time = 0"                4 0 200 200 200 5 10 fifo
check_invalid "burnout_time negatif"            4 -1 200 200 200 5 10 fifo
check_invalid "compile_time negatif"            4 800 -1 200 200 5 10 fifo
check_invalid "debug_time negatif"              4 800 200 -1 200 5 10 fifo
check_invalid "refactor_time negatif"           4 800 200 200 -1 5 10 fifo
check_invalid "nb_compiles = 0"                 4 800 200 200 200 0 10 fifo
check_invalid "nb_compiles negatif"             4 800 200 200 200 -1 10 fifo
check_invalid "dongle_cooldown negatif"         4 800 200 200 200 5 -1 fifo
check_invalid "scheduler inconnu"               4 800 200 200 200 5 10 azerty
check_invalid "texte au lieu d'un nombre"       abc 800 200 200 200 5 10 fifo
check_invalid "nombre hors limites (overflow)"  4 99999999999999999999 200 200 200 5 10 fifo
check_invalid "nb_coders > INT_MAX"             99999999999 800 200 200 200 5 10 fifo

# ---------------------------------------------------------------- #
# 2. Cas fonctionnels
# ---------------------------------------------------------------- #

info "2. Cas fonctionnels (doivent tourner sans crash)"

check_runs() {
	local desc="$1"
	local logfile="$2"
	shift 2
	local out
	out=$(timeout 20 "$BIN" "$@" 2>&1)
	local ret=$?
	if [ -n "$logfile" ]; then
		printf '%s\n' "$out" > "$logfile"
	fi
	if [ $ret -eq 124 ]; then
		ko "$desc (timeout : ne s'est jamais arrete)"
	elif [ $ret -ge 128 ]; then
		ko "$desc (crash detecte, code=$ret)"
	else
		ok "$desc (retour=$ret)"
	fi
}

check_runs "1 seul coder (doit finir par burnout)" /tmp/codexion_1coder.log \
	1 800 200 200 200 5 10 fifo
grep -q "burned out" /tmp/codexion_1coder.log \
	&& ok "1 seul coder -> burnout bien detecte" \
	|| ko "1 seul coder -> aucun burnout detecte"
grep -q "is compiling" /tmp/codexion_1coder.log \
	&& ko "1 seul coder -> ne devrait jamais compiler (1 seul dongle)" \
	|| ok "1 seul coder -> ne compile jamais (normal, 1 seul dongle)"

check_runs "2 coders (cas minimal fonctionnel)" "" \
	2 800 200 200 200 5 10 fifo

check_runs "Nombre impair de coders (5), scheduler edf" "" \
	5 800 200 200 200 7 10 edf

check_runs "Simulation qui doit reussir (temps larges)" /tmp/codexion_ok.log \
	4 800 200 200 200 5 10 fifo
grep -q "burned out" /tmp/codexion_ok.log \
	&& ko "Simulation avec temps larges -> ne devrait pas burnout" \
	|| ok "Simulation avec temps larges -> pas de burnout"

check_runs "Simulation qui doit burnout (temps trop courts)" /tmp/codexion_burn.log \
	4 100 200 200 200 5 10 fifo
grep -q "burned out" /tmp/codexion_burn.log \
	&& ok "Burnout provoque volontairement -> bien detecte" \
	|| ko "Burnout provoque volontairement -> non detecte"
lines_after_burn=$(awk '/burned out/{f=1;next} f' /tmp/codexion_burn.log | wc -l)
[ "$lines_after_burn" -eq 0 ] \
	&& ok "Aucun message affiche apres le burnout" \
	|| ko "Des messages sont affiches apres le burnout ($lines_after_burn lignes)"

# Grand temps de compilation + burnout court : on ne doit PAS attendre
# toute la duree du compile, l'arret doit etre quasi immediat.
start_ms=$(date +%s%3N)
timeout 10 "$BIN" 2 200 5000 100 100 5 0 fifo > /tmp/codexion_long.log 2>&1
ret=$?
elapsed_ms=$(( $(date +%s%3N) - start_ms ))
if [ $ret -eq 124 ] || [ "$elapsed_ms" -gt 1500 ]; then
	ko "Long compile + burnout court : arret trop lent (${elapsed_ms} ms)"
else
	ok "Long compile + burnout court : arret rapide (${elapsed_ms} ms)"
fi

check_runs "Beaucoup de coders (50)" "" \
	50 800 200 200 200 3 10 fifo

# ---------------------------------------------------------------- #
# 3. Format des logs
# ---------------------------------------------------------------- #

info "3. Format des logs"

timeout 20 "$BIN" 4 800 200 200 200 5 10 fifo > /tmp/codexion_fmt.log 2>&1

FMT='^[0-9]+ [0-9]+ (is compiling|is debugging|is refactoring|has taken a dongle|burned out)$'
if grep -vE "$FMT" /tmp/codexion_fmt.log | grep -q .; then
	ko "Toutes les lignes respectent le format 'temps id message'"
	grep -vE "$FMT" /tmp/codexion_fmt.log | head -5
else
	ok "Toutes les lignes respectent le format 'temps id message'"
fi

# Chaque compile suppose 2 dongles pris. A l'arret, un coder peut en
# avoir pris un seul de plus, donc on verifie taken >= 2 * compiling.
count_taken=$(grep -c "has taken a dongle" /tmp/codexion_fmt.log)
count_comp=$(grep -c "is compiling" /tmp/codexion_fmt.log)
if [ "$count_taken" -ge $((2 * count_comp)) ]; then
	ok "'has taken a dongle' ($count_taken) >= 2 x 'is compiling' ($count_comp)"
else
	ko "'has taken a dongle' ($count_taken) < 2 x 'is compiling' ($count_comp)"
fi

# Les timestamps doivent commencer pres de 0 (temps relatif) et ne
# jamais decroitre.
first_ts=$(head -1 /tmp/codexion_fmt.log | cut -d' ' -f1)
if [ -n "$first_ts" ] && [ "$first_ts" -lt 1000 ] 2> /dev/null; then
	ok "Timestamps relatifs au demarrage (premier = $first_ts ms)"
else
	ko "Timestamps non relatifs au demarrage (premier = $first_ts)"
fi

# ---------------------------------------------------------------- #
# 4. Repetition
# ---------------------------------------------------------------- #

info "4. Execution repetee (detection de bugs aleatoires)"

repeat_fail=0
for i in $(seq 1 15); do
	timeout 20 "$BIN" 4 800 200 200 200 5 10 fifo > /dev/null 2>&1
	ret=$?
	if [ $ret -ne 0 ]; then
		repeat_fail=$((repeat_fail + 1))
	fi
done
if [ $repeat_fail -eq 0 ]; then
	ok "15 executions consecutives, toutes en retour 0"
else
	ko "15 executions consecutives : $repeat_fail ont echoue"
fi

# ---------------------------------------------------------------- #
# 5. Valgrind
# ---------------------------------------------------------------- #

info "5. Valgrind (fuites memoire)"

if command -v valgrind > /dev/null 2>&1; then
	timeout 60 valgrind --leak-check=full --show-leak-kinds=all \
		--error-exitcode=42 \
		"$BIN" 4 800 200 200 200 5 10 fifo > /tmp/codexion_valgrind.log 2>&1
	if grep -qE "ERROR SUMMARY: 0 errors" /tmp/codexion_valgrind.log \
		&& ! grep -qE "(definitely|indirectly) lost: [1-9]" /tmp/codexion_valgrind.log; then
		ok "Valgrind : aucune fuite memoire (0 erreur, 0 octet perdu)"
	else
		ko "Valgrind : fuite memoire detectee (voir /tmp/codexion_valgrind.log)"
	fi
else
	echo "valgrind non installe : test ignore."
fi

# ---------------------------------------------------------------- #
# 6. ThreadSanitizer
# ---------------------------------------------------------------- #

info "6. ThreadSanitizer (data races)"

# Sources dans src/ si ce dossier existe, sinon dans le dossier courant
if ls src/*.c > /dev/null 2>&1; then
	TSAN_SRC=(src/*.c)
	TSAN_INC="-I src"
else
	TSAN_SRC=(*.c)
	TSAN_INC=""
fi

if cc -fsanitize=thread -pthread -x c -o /tmp/tsan_check -c /dev/null 2> /dev/null \
	&& cc -Wall -Wextra -pthread -fsanitize=thread -g $TSAN_INC \
		"${TSAN_SRC[@]}" -o /tmp/codexion_tsan 2> /tmp/tsan_build.log; then
	TSAN_OPTIONS="halt_on_error=0" timeout 20 /tmp/codexion_tsan \
		4 800 200 200 200 5 10 fifo > /tmp/codexion_tsan.log 2>&1
	if grep -q "WARNING: ThreadSanitizer" /tmp/codexion_tsan.log; then
		ko "ThreadSanitizer : data race detectee (voir /tmp/codexion_tsan.log)"
	else
		ok "ThreadSanitizer : aucune data race detectee"
	fi
	rm -f /tmp/codexion_tsan
else
	echo "ThreadSanitizer indisponible ou compilation echouee (voir /tmp/tsan_build.log) : test ignore."
fi

# ---------------------------------------------------------------- #
# Resume final
# ---------------------------------------------------------------- #

echo ""
info "Resume"
echo -e "${GREEN}Reussis : $PASS${NC}"
echo -e "${RED}Echoues : $FAIL${NC}"
if [ $FAIL -gt 0 ]; then
	echo "Tests en echec :"
	for t in "${FAILED_TESTS[@]}"; do
		echo "  - $t"
	done
	echo ""
	print_box "$RED" \
		"😭  REESSAYE ET N'ABANDONNE JAMAIS  😭" \
		"" \
		"💀 $FAIL test(s) en echec 💀" \
		"Regarde la liste des erreurs au-dessus et debogue-les 😢🙏"
	exit 1
fi

echo ""
print_box "$MAGENTA" \
	"🎉  CONGRATULATIONS, YOU PASSED ALL THE TESTS  🎉" \
	"" \
	"😂🤣😂  $PASS / $PASS You did it  😂🤣😂" \
	"Keep going and do the next step ! 🔥🚀"
exit 0
