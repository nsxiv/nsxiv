#!/bin/sh -e

std="c99"

run_cppcheck() {
    cppcheck --std="$std" --enable=performance,portability \
        --force --quiet --inline-suppr --error-exitcode=1 \
        --max-ctu-depth=8 -j"$(nproc)" \
        $(make OPT_DEP_DEFAULT="$1" dump_cppflags) \
        --suppress=varFuncNullUB --suppress=uninitvar \
        *.c
}

run_tidy() {
    checks="$(sed '/^#/d' etc/woodpecker/clang-tidy-checks | paste -d ',' -s)"
    clang-tidy --warnings-as-errors="*" --checks="$checks" --quiet *.c \
        -- -std="$std" $(make OPT_DEP_DEFAULT="$1" dump_cppflags)
}

run_cppcheck "0"; run_cppcheck "1";
run_tidy "0"; run_tidy "1";
