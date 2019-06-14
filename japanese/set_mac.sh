brew link --overwrite gcc@8
find . -type f -name '*.c' -or -name '*.h'|xargs -n1 nkf -Sw --cp932 --overwrite
sh sys/unix/setup.sh sys/unix/hints/macosx10.10
