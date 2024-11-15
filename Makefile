simulation:
	clear
	g++ -std=c++20 -O3 simulation.cpp -o simulation.out
	./simulation.out

viali: 
	clear
	cmake -B build -DCMAKE_BUILD_TYPE=Release && make -C build
	./viali.out

gifter:
	clear
	python ../dsf/utils/gifter.py --adj-matrix ./datas69_op/adj.dat --coordinates ./datas69_op/coords.csv --densities ./datas69_op/densities.csv