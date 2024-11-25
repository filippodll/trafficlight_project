viali: 
	clear
	cmake -B build -DCMAKE_BUILD_TYPE=Release && make -C build
	./viali_real.out 69 2024-06-10 300 1 ./signal 0
viali_debug: 
	clear
	cmake -B build -DCMAKE_BUILD_TYPE=Debug && make -C build
	./viali_real.out 69 2024-06-10 300 1 ./signal 0
gifter:
	clear
	curl ascii.live/rick
video:
	clear
	python ../DynamicalSystemFramework/utils/videomaker.py --densities ./2023-05-30/densities.csv --day 2023-05-30 --fps 3 --use-basemap 1 --adj-matrix ./constants/adj.dat --coordinates ./constants/coords.csv
study:
	python study.py --day 2024-06-28 --n-nodes 21 --input-folder ./signal