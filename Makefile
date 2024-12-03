viali: 
	clear
	cmake -B build -DCMAKE_BUILD_TYPE=Release && make -C build
	./viali.out 69 2023-05-07 300 1 ./signal 100 0 0.35 0.7
	./viali.out 69 2023-05-07 300 1 ./signal 100 1 0.15 0
	./viali.out 69 2023-05-07 300 1 ./signal 100 2 0.35 0.7
viali_debug: 
	clear
	cmake -B build -DCMAKE_BUILD_TYPE=Debug && make -C build
	./viali.out 69 2024-06-06 300 1 ./jun24 0
gifter:
	clear
	curl ascii.live/rick
video:
	clear
	python ../DynamicalSystemFramework/utils/videomaker.py --densities ./2023-05-26/densities.csv --day 2023-05-26 --fps 5 --use-basemap 1 --adj-matrix ./constants/adj.dat --coordinates ./constants/coords.csv
study:
	python study.py --day 2023-05-07 --n-nodes 21 --input-folder ./signal