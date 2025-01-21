import argparse
import subprocess


def cmd(command):
    try:
        subprocess.run(command, check=True, shell=True)
    except subprocess.CalledProcessError as e:
        print(f"Error executing command: {command}\n{e}")
        exit(1)


if __name__ == "__main__":
    # take in input one string and one double
    parser = argparse.ArgumentParser(description="Viali")
    parser.add_argument("-d", "--date", required=True, type=str, help="Date")
    parser.add_argument(
        "-t", "--threshold", required=True, type=float, help="Threshold"
    )
    args = parser.parse_args()
    cmd("clear")
    cmd("cmake -B build -DCMAKE_BUILD_TYPE=Release && make -C build")
    cmd(f"./viali.out 69 0.6 {args.date} 300 1 ./signal 100 0 {args.threshold}")
    cmd(f"./viali.out 69 0.6 {args.date} 300 1 ./signal 100 1 {args.threshold}")
    cmd(f"./viali.out 69 0.6 {args.date} 300 1 ./signal 100 2 {args.threshold}")
    cmd(f"python study.py --day {args.date} --n-nodes 21 --input-folder ./signal")
