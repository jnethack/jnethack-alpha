brew link --overwrite gcc@9
find . -type f -name '*.c' -or -name '*.h'|xargs -n1 nkf -Sw --cp932 --overwrite
find dat -type f|xargs -n1 nkf -Sw --cp932 --overwrite
sh sys/unix/setup.sh sys/unix/hints/macosx10.14
