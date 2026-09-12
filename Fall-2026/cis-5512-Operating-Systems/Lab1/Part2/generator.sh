gcc -DPRINT -o isort isort.c
gcc -DPRINT -o isort2 isort2.c

./isort > output1.txt
./isort2 > output2.txt

diff output1.txt output2.txt