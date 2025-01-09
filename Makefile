viali: 
	clear
	cmake -B build -DCMAKE_BUILD_TYPE=Release && make -C build
	echo "\nStarting normal simulation\n"
	./viali.out 69 0.95 2023-05-11 300 1 ./signal 100 0 0.35 1
	echo "\nStarting simulation with optimization strategy one\n"
	./viali.out 69 0.95 2023-05-11 300 1 ./signal 100 1 0.65 0.3
	echo "\nStarting simulation with optimization strategy two\n"
	./viali.out 69 0.95 2023-05-11 300 1 ./signal 100 2 0.65 1
viali_debug: 
	clear
	cmake -B build -DCMAKE_BUILD_TYPE=Debug && make -C build
	./viali.out 69 2024-06-06 300 1 ./jun24 0
gifter:
	clear
	curl ascii.live/rick
video:
	clear
	python ../DynamicalSystemFramework/utils/videomaker.py --densities ./output/2024-06-21/densities.csv --day 2024-06-21 --fps 5 --use-basemap 1 --adj-matrix ./constants/adj.dat --coordinates ./constants/coords.csv
study:
	python study.py --day 2023-05-11 --n-nodes 21 --input-folder ./signal