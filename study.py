import argparse
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from structures_out import COIL_DICT

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--day", type=str, required=True, help="Day to study in the format YYYY-MM-DD"
    )
    parser.add_argument(
        "--n-nodes", type=int, required=True, help="Number of nodes in the network"
    )
    parser.add_argument("--input-folder", type=str, required=True, help="Input folder")
    args = parser.parse_args()

    df_real = pd.read_csv(f"{args.input_folder}/{args.day}.csv", sep=";")
    df_synth = pd.read_csv(f"./{args.day}/out_spires.csv", sep=";")

    OUTPUT_COILS = [21, 30, 76, 155, 166]
    INPUT_COILS = [1, 175, 190, 211, 233, 254, 276, 297, 341, 363, 384, 427]
    INNER_COILS = [
        c for c in COIL_DICT.values() if c not in OUTPUT_COILS and c not in INPUT_COILS
    ]

    print(f"Day {args.day}\n")

    tot_input = 0
    tot_output = 0
    tot_inner = 0
    tot_input_synth = 0
    tot_output_synth = 0
    tot_inner_synth = 0

    df_diff = pd.DataFrame()
    df_diff["time"] = np.convolve(
        df_synth["time"].to_list(), np.ones((12,)) / 12, mode="full"
    )
    # df_diff["time"] = df_synth["time"]
    # restrict df_diff in 8*12:20*12
    df_diff = df_diff[8 * 12 : 20 * 12]
    df_in = pd.DataFrame()

    df_validation = pd.DataFrame()
    df_validation["time"] = df_synth["time"]
    df_validation["input"] = 0
    df_validation["output"] = 0

    for _, row in df_real.iterrows():
        idx = COIL_DICT.get(row["section"].strip())
        data_real = [int(d) for d in row["data"].split()]
        data_synth = df_synth[str(idx)].to_list()

        if int(idx) in INPUT_COILS:
            df_in[idx] = data_real
            tot_input += sum(data_real)
            tot_input_synth += df_synth[str(idx)].sum()
            df_validation["input"] += data_real
        elif int(idx) in OUTPUT_COILS:
            data_synth = df_synth[str(idx)].to_list()
            tot_output += sum(data_real)
            tot_output_synth += sum(data_synth)
            df_validation["output"] += data_real

            # compute difference
            if len(data_real) == len(data_synth):
                mean_diff = None

                print(
                    f"Coil {idx // args.n_nodes} -> {idx % args.n_nodes}: {sum(data_real)} vs {sum(data_synth)}"
                )

                data_real = np.convolve(data_real, np.ones((12,)) / 12, mode="full")
                data_synth = np.convolve(data_synth, np.ones((12,)) / 12, mode="full")
                diff = np.subtract(data_real, data_synth)
                if mean_diff is None:
                    mean_diff = diff
                else:
                    mean_diff += diff
                # substitue zeros with one in data_real
                diff = diff / np.array([1 if d == 0 else d for d in data_real])
                diff = diff[8 * 12 : 20 * 12] * 100
                df_diff[str(idx)] = diff
                # diff = diff * 65 / 85
                # x labels: one point every 5 minutes from 8:00 to 20:00
                x = np.arange(8 * 12, 20 * 12)
                # convert into time
                x = [f"{int(t // 12):02d}:{int(t % 12) * 5:02d}" for t in x]
                plt.plot(
                    x, diff, label=f"Coil {idx // args.n_nodes} -> {idx % args.n_nodes}"
                )

        elif int(idx) in INNER_COILS:
            data_synth = df_synth[str(idx)].to_list()
            tot_inner += sum(data_real)
            tot_inner_synth += sum(data_synth)

    df_diff = df_diff.set_index("time")
    for id in df_diff.columns:
        print(
            f"Coil {int(id) // args.n_nodes} -> {int(id) % args.n_nodes} - Mean relative error: {df_diff[id].mean(axis=0):.2f}%"
        )
    df_diff["mean"] = df_diff.mean(axis=1)
    print(f"Total mean relative error: {df_diff["mean"].mean(axis=0):.2f}%")
    # df_diff.to_csv(f"{args.day}-diff.csv", index=False, sep=";")

    print()
    print(f"Total input: {tot_input}, Total output: {tot_output}")
    print(
        f"Total input synth (coils): {tot_input_synth}, Total output synth: {tot_output_synth}"
    )
    print(f"Total inner: {tot_inner}")
    print(f"Total inner synth: {tot_inner_synth}")

    plt.title(
        f"{args.day}\nDifference between output REAL and SIMULATED data - media oraria"
    )
    # show only one in 12 ticks
    plt.xticks(np.arange(0, len(df_diff), 6), rotation=45)
    # make a grid with dashed lines
    plt.grid(linestyle="--")
    plt.xlabel("Simulation time")
    plt.ylabel("Relative error (%)")
    plt.legend()
    plt.show()

    # df_validation.set_index("time")
    # df_validation["delta"] = df_validation["input"] - df_validation["output"]
    # plt.plot(df_validation["delta"], label="Delta")
    # plt.plot(df_validation["input"], label="Input")
    # plt.plot(df_validation["output"], label="Output")
    # plt.legend()
    # plt.show()

    # plt.plot(mean_diff, label="Mean difference")
    # plt.show()

    df_data = pd.read_csv(f"./{args.day}/data.csv", sep=";")
    # if exists ./{args.day}-optimized/data.csv import it
    df_opt = None
    try:
        df_opt = pd.read_csv(f"./{args.day}-optimized/data.csv", sep=";")
    except FileNotFoundError:
        pass

    # # plot mean_traveltime over mean_density for df_data
    # plt.scatter(df_data["mean_density"], df_data["mean_traveltime"], label="Normal")
    # if df_opt is not None:
    #     plt.scatter(
    #         df_opt["mean_density"], df_opt["mean_traveltime"], label="Optimized"
    #     )
    # plt.xlabel("Mean density")
    # plt.ylabel("Mean travel time")
    # plt.legend()
    # plt.show()
