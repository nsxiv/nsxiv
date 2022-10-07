#!/bin/sh
#
# Shell script that checks for all possible build combination with TCC.
# Usage: call the script while in the nsxiv root directory

set -- $(grep -o '^HAVE_[[:alpha:]]* ' config.mk)
CFLAGS="$(sed '/^#/d' etc/woodpecker/CFLAGS | paste -d ' ' -s)"
z=$(echo "2 ^ $#" | bc)

print_opt_name() {
    shift "$(( $1 + 1 ))"
    printf "%s=" "$1"
}

print_opt_arg() {
    bn=$(echo "$1 / (2 ^ $2)" | bc)
    printf "%d " $(( bn % 2 ))
}

n=0
while [ "$n" -lt "$z" ]; do
    i=0
    while [ "$i" -lt "$#" ]; do
        print_opt_name "$i" "$@"
        print_opt_arg "$n" "$i"
        i=$((i + 1))
    done | tee "/dev/stderr" | (
        make clean
        if ! xargs make -j"$(nproc)" CC=tcc CFLAGS="$CFLAGS" LDFLAGS="$CFLAGS"; then
            echo "[FAILED]" >&2
            exit 1
        else
            echo "[SUCCESS]" >&2
        fi
    )
    [ "$?" -ne 0 ] && exit "$?"
    n=$((n + 1))
done >/dev/null
