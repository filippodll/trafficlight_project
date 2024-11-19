simulation:
	clear
	g++ -std=c++20 -O3 simulation.cpp -o simulation.out
	./simulation.out

viali: 
	clear
	cmake -B build -DCMAKE_BUILD_TYPE=Release && make -C build
	./viali_real.out 69 2023-05-13 300 1 ./may23
gifter:
	clear
	curl ascii.live/rick
video:
	clear
	python ../dsf/utils/videomaker.py --densities ./2023-05-12/densities.csv --day 2023-05-12 --use-basemap 1 --adj-matrix ./constants/adj.dat --coordinates ./constants/coords.csv
study:
	python study.py --day 2023-05-13 --n-nodes 21 --input-folder ./may23