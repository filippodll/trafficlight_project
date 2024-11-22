simulation:
	clear
	g++ -std=c++20 -O3 simulation.cpp -o simulation.out
	./simulation.out

viali: 
	clear
	cmake --preset=release
	cmake --build build
	./viali_real.out 69 2023-05-07 300 0 ./may23 0
viali_debug: 
	clear
	cmake --preset=debug
	cmake --build build
	./viali_real.out 69 2023-05-03 300 1 ./may23 0
gifter:
	clear
	curl ascii.live/rick
video:
	clear
	python ../DynamicalSystemFramework/utils/videomaker.py --densities ./2023-05-12/densities.csv --day 2023-05-12 --fps 2 --use-basemap 1 --adj-matrix ./constants/adj.dat --coordinates ./constants/coords.csv
study:
	python study.py --day 2023-05-07 --n-nodes 21 --input-folder ./may23