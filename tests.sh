#!/bin/bash
# ****************************************************************** #
#  Manou_tests.sh                                                    #
#  By: Manou(nfitahin)                                               #
#                                                                    #
#  Script de tests automatiques pour le projet Codexion.             #
#  A placer dans le meme dossier que le Makefile / codexion.c, puis :#
#                                                                    #
#      chmod +x Manou_tests.sh                                       #
#      ./Manou_tests.sh                                              #
#                                                                    #
#  Le script compile le projet, lance une serie de tests (arguments  #
#  invalides, cas fonctionnels, format des logs, memoire, races),    #
#  et affiche un resume PASS/FAIL a la fin.                          #
# ****************************************************************** #

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

# liste des emojis utilises dans les bannieres : chacun occupe 2
# colonnes a l'ecran mais compte pour 1 seul "caractere" en bash,
# d'ou le cadre qui partait de travers. On corrige ca dans
# visual_width() en comptant +1 colonne par emoji trouve.
EMOJI_REGEX='💻|🚀|🎉|😂|🤣|🔥|😭|💀|🙏|😢'

# visual_width str -> largeur affichee reelle (tient compte du fait
# qu'un emoji prend 2 colonnes a l'ecran, pas 1).
visual_width() {
	local s="$1"
	local charlen=${#s}
	local ecount
	ecount=$(printf '%s' "$s" | grep -oE "$EMOJI_REGEX" 2> /dev/null | wc -l)
	echo $((charlen + ecount))
}

# repeat_char c n -> affiche le caractere (multi-octet compris) c
# repete n fois (n peut etre 0). Une boucle plutot que tr, car tr ne
# gere pas bien les caracteres UTF-8 multi-octets comme ═.
repeat_char() {
	local char="$1"
	local n="$2"
	local i=0
	while [ "$i" -lt "$n" ]; do
		printf '%s' "$char"
		i=$((i + 1))
	done
}

# print_box color ligne1 ligne2 ... -> affiche une boite centree autour
# du texte (largeur calculee automatiquement sur la ligne la plus
# longue, en tenant compte de la largeur reelle a l'ecran).
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

# ---------------------------------------------------------------- #
# Banniere de depart, signature Manou
# ---------------------------------------------------------------- #

print_box "$CYAN" \
	"💻  CODEXION - TEST SUITE  💻" \
	"" \
	"Ecrit par Manou(nfitahin) @ 42 Antananarivo" \
	"🚀 On lance les tests, tsy misy tahotra ! 🚀"

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
# 1. Arguments invalides : doivent echouer proprement (pas de crash,
#    pas de simulation lancee, code de retour != 0 sauf cas "aucun
#    argument" qui doit renvoyer 0 sans rien afficher).
# ---------------------------------------------------------------- #

info "1. Arguments invalides"

check_invalid() {
	local desc="$1"
	shift
	local out
	out=$(timeout 2 "$BIN" "$@" 2>&1)
	local ret=$?
	if [ $ret -eq 0 ] && [ -z "$out" ]; then
		ko "$desc (devrait afficher une erreur ou tourner, retour=$ret)"
	elif [ $ret -ne 0 ]; then
		ok "$desc (retour=$ret, rejete correctement)"
	else
		ko "$desc (retour=$ret, sortie inattendue: $out)"
	fi
}

# aucun argument -> doit renvoyer 0 et ne RIEN afficher
out=$(timeout 1 "$BIN" 2>&1)
ret=$?
if [ $ret -eq 0 ] && [ -z "$out" ]; then
	ok "Aucun argument -> return 0, aucune sortie"
else
	ko "Aucun argument -> return 0, aucune sortie (retour=$ret, sortie: $out)"
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

# ---------------------------------------------------------------- #
# 2. Cas fonctionnels : la simulation doit tourner sans crash et
#    se terminer avec le bon code de retour.
# ---------------------------------------------------------------- #

info "2. Cas fonctionnels (doivent tourner sans crash)"

check_runs() {
	local desc="$1"
	local logfile="$2"
	shift 2
	local out
	out=$(timeout 10 "$BIN" "$@" 2>&1)
	local ret=$?
	if [ -n "$logfile" ]; then
		printf '%s\n' "$out" > "$logfile"
	fi
	if [ $ret -eq 124 ]; then
		ko "$desc (timeout : ne s'est jamais arrete)"
	elif [ $ret -eq 139 ] || [ $ret -eq 134 ] || [ $ret -eq 136 ]; then
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

check_runs "Beaucoup de coders (50)" "" \
	50 800 200 200 200 3 10 fifo

# ---------------------------------------------------------------- #
# 3. Format des logs
# ---------------------------------------------------------------- #

info "3. Format des logs"

"$BIN" 4 800 200 200 200 5 10 fifo > /tmp/codexion_fmt.log 2>&1

if grep -vE '^[0-9]+ [0-9]+ (is compiling|is debugging|is refactoring|has taken a dongle|burned out)$' \
	/tmp/codexion_fmt.log | grep -q .; then
	ko "Toutes les lignes respectent le format 'temps id message'"
	grep -vE '^[0-9]+ [0-9]+ (is compiling|is debugging|is refactoring|has taken a dongle|burned out)$' \
		/tmp/codexion_fmt.log
else
	ok "Toutes les lignes respectent le format 'temps id message'"
fi

count_taken=$(grep -c "has taken a dongle" /tmp/codexion_fmt.log)
if [ $((count_taken % 2)) -eq 0 ]; then
	ok "'has taken a dongle' apparait un nombre pair de fois ($count_taken)"
else
	ko "'has taken a dongle' apparait un nombre IMPAIR de fois ($count_taken)"
fi

# ---------------------------------------------------------------- #
# 4. Repetition (detecte les bugs qui n'apparaissent qu'aleatoirement)
# ---------------------------------------------------------------- #

info "4. Execution repetee (detection de bugs aleatoires)"

repeat_fail=0
for i in $(seq 1 15); do
	timeout 5 "$BIN" 4 800 200 200 200 5 10 fifo > /dev/null 2>&1
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
# 5. Valgrind (fuites memoire) - seulement si installe
# ---------------------------------------------------------------- #

info "5. Valgrind (fuites memoire)"

if command -v valgrind > /dev/null 2>&1; then
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=42 \
		"$BIN" 4 800 200 200 200 5 10 fifo > /tmp/codexion_valgrind.log 2>&1
	# Valgrind affiche soit "definitely/indirectly lost: 0 bytes" (leak
	# summary complet), soit directement "no leaks are possible" quand
	# tout a ete libere. On accepte les deux formes, et on echoue des
	# qu'un nombre != 0 apparait sur une ligne "lost".
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
# 6. ThreadSanitizer (data races) - seulement si le compilateur le supporte
# ---------------------------------------------------------------- #

info "6. ThreadSanitizer (data races)"

if cc -fsanitize=thread -pthread -x c -o /tmp/tsan_check -c /dev/null 2>/dev/null \
	&& cc -Wall -Wextra -pthread -fsanitize=thread -g *.c -o /tmp/codexion_tsan 2>/tmp/tsan_build.log; then
	TSAN_OPTIONS="halt_on_error=0" timeout 5 /tmp/codexion_tsan \
		4 800 200 200 200 5 10 fifo > /tmp/codexion_tsan.log 2>&1
	if grep -q "WARNING: ThreadSanitizer" /tmp/codexion_tsan.log; then
		ko "ThreadSanitizer : data race detectee (voir /tmp/codexion_tsan.log)"
	else
		ok "ThreadSanitizer : aucune data race detectee"
	fi
	rm -f /tmp/codexion_tsan
else
	echo "ThreadSanitizer non disponible sur ce compilateur/systeme : test ignore."
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
		"😭  DOMAZY BE HO ANAOU  😭" \
		"" \
		"💀 $FAIL test(s) en echec 💀" \
		"Regarde la liste au-dessus courage 😢🙏"
	exit 1
fi

echo ""
print_box "$MAGENTA" \
	"🎉  ARAHABAINA FA TSSSSS  🎉" \
	"" \
	"😂🤣😂  $PASS / $PASS tena manao nareo ka  😂🤣😂" \
	"Zay fotsiny dia vita aa ! 🔥🚀"
exit 0
