simulation:
	clear
	g++ -std=c++20 -O3 simulation.cpp -o simulation.out
	./simulation.out

viali: 
	clear
	cmake -B build -DCMAKE_BUILD_TYPE=Release && make -C build
	./viali.out