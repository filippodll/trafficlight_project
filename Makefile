simulation:
	clear
	g++ -std=c++20 -O3 simulation.cpp -o simulation.out
	./simulation.out

viali: 
	clear
	cmake -B build -DCMAKE_BUILD_TYPE=Release && make -C build
	./viali_real.out

gifter:
	clear
	rm -r temp_img/*.png
	python ../dsf/utils/gifter.py --adj-matrix ./2024-06-06/adj.dat --coordinates ./2024-06-06/coords.csv --densities ./2024-06-06/densities.csv --use-basemap 1 --n-frames 20 --time-begin 0 --day 2024-06-06