#!/bin/sh -e

std="c99"
NProc=$(( $(nproc) / 4 ))
if [ -z "$NProc" ] || [ "$NProc" -lt 1 ]; then NProc="1"; fi

run_cppcheck() {
    cppcheck --std="$std" --enable=performance,portability \
        --force --quiet --inline-suppr --error-exitcode=1 \
        --max-ctu-depth=8 -j"$NProc" \
        $(make OPT_DEP_DEFAULT="$1" dump_cppflags) -DDEBUG \
        --suppress=varFuncNullUB --suppress=uninitvar \
        $(git ls-files *.c)
}

run_tidy() {
    checks="$(sed '/^#/d' etc/woodpecker/clang-tidy-checks | paste -d ',' -s)"
    git ls-files *.c | xargs -P"$NProc" -I{} clang-tidy --quiet \
        --warnings-as-errors="*" --checks="$checks" {} \
        -- -std="$std" $(make OPT_DEP_DEFAULT="$1" dump_cppflags) -DDEBUG
}

run_cppcheck "0" & run_cppcheck "1" & run_tidy "0" & run_tidy "1";
wait
