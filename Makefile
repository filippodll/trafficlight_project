viali: 
	python viali.py -d 2024-06-03 -t 30 -r
viali_debug: 
	clear
	cmake -B build -DCMAKE_BUILD_TYPE=Debug && make -C build
	./viali.out 69 2024-06-06 300 1 ./jun24 0
gifter:
	clear
	curl ascii.live/rick
video:
	clear
	python ../DynamicalSystemFramework/utils/videomaker.py --densities ./output/2024-06-06/densities.csv --day 2024-06-06 --fps 5 --use-basemap 1 --adj-matrix ./constants/adj.dat --coordinates ./constants/coords.csv