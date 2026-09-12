#!/bin/bash

cp matrix.c ijk.c 
sed -i s/#1/i/g ijk.c 
sed -i s/#2/j/g ijk.c 
sed -i s/#3/k/g ijk.c 



cp matrix.c ikj.c 
sed -i s/#1/i/g ikj.c 
sed -i s/#2/k/g ikj.c 
sed -i s/#3/j/g ikj.c


cp matrix.c jik.c 
sed -i s/#1/j/g jik.c 
sed -i s/#2/i/g jik.c 
sed -i s/#3/k/g jik.c


cp matrix.c jki.c 
sed -i s/#1/j/g jki.c 
sed -i s/#2/k/g jki.c 
sed -i s/#3/i/g jki.c


cp matrix.c kij.c 
sed -i s/#1/k/g kij.c 
sed -i s/#2/i/g kij.c 
sed -i s/#3/j/g kij.c


cp matrix.c kji.c 
sed -i s/#1/k/g kji.c 
sed -i s/#2/j/g kji.c 
sed -i s/#3/i/g kji.c


orders=(ijk ikj jik jki kij kji)

for order in "${orders[@]}"
do 
	gcc -DN=5 -DRUNS=1 -DPRINT -o "$order" "$order.c"
	./"$order"
done



# ------------------------------------------------

sizes=(1000 2000 3000)
RUNS=3

output="results.csv"

echo "N, LoopOrder, AverageElapsedTime, Performance" > "$output"


for N in "${sizes[@]}"
do 
	for order in "${orders[@]}"
	do 
		gcc -DN=$N -DRUNS=$RUNS -o "$order" "$order.c"
	
		result=$(./"$order")
		
	
		avg_elapsed_time=$(echo "$result" | awk '{print $1}')
		performance=$(echo "$result" | awk '{print $2}')
	
		echo "$N, $order, $avg_elapsed_time, $performance" >> "$output"
		
		echo 
	done 
	
done