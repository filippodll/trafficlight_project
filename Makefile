simulation:
	clear
	g++ -std=c++20 -O3 simulation.cpp -o simulation.out
	./simulation.out

viali: 
	clear
	cmake -B build -DCMAKE_BUILD_TYPE=Release && make -C build
	./viali_real.out 69 2023-05-25 300 1 ./may23
gifter:
	clear
	curl ascii.live/rick
video:
	clear
	python ../dsf/utils/videomaker.py --adj-matrix ./2023-05-25/adj.dat --coordinates ./2023-05-25/coords.csv --densities ./2023-05-25/densities.csv --use-basemap 1 --day 2023-05-25