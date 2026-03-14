TEMP=1

for i in $opcoes ; do 
    if (( i == 8 )); then
        ./stats.sh
        so_success S4.6
        TEMP=0
        break
    fi
done

if (( TEMP == 1 )); then 
    ./stats.sh $opcoes
    so_success S4.6
fi 