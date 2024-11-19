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

    tot_input = 0
    tot_output = 0
    tot_inner = 0
    tot_input_synth = 0
    tot_output_synth = 0
    tot_inner_synth = 0

    df_diff = pd.DataFrame()
    df_diff["time"] = df_synth["time"]
    df_in = pd.DataFrame()

    for _, row in df_real.iterrows():
        idx = COIL_DICT.get(row["section"].strip())
        data_real = [int(d) for d in row["data"].split()]
        data_synth = df_synth[str(idx)].to_list()
        if int(idx) in INPUT_COILS:
            df_in[idx] = data_real
            tot_input += sum(data_real)
            tot_input_synth += df_synth[str(idx)].sum()
        elif int(idx) in OUTPUT_COILS:
            data_synth = df_synth[str(idx)].to_list()
            tot_output += sum(data_real)
            tot_output_synth += sum(data_synth)

        elif int(idx) in INNER_COILS:
            data_synth = df_synth[str(idx)].to_list()
            tot_inner += sum(data_real)
            tot_inner_synth += sum(data_synth)

            mean_diff = None

            # compute difference
            if len(data_real) == len(data_synth):
                diff = np.subtract(
                    data_real, data_synth
                )  # [d * 65672 / 85619 for d in data_real]
                if mean_diff is None:
                    mean_diff = diff
                else:
                    mean_diff += diff
                df_diff[str(idx)] = diff
                diff = diff / data_real
                # limit x after 8 am, every point are 5 minutes
                # mobile mean every 12 points
                diff = diff[8 * 12 :]
                diff = np.convolve(diff, np.ones((12,)) / 12, mode="valid")
                plt.plot(
                    diff, label=f"Coil {idx // args.n_nodes} -> {idx % args.n_nodes}"
                )

    df_diff = df_diff.set_index("time")
    df_diff["sum"] = df_diff.sum(axis=1)
    df_diff.to_csv(f"{args.day}-diff.csv", index=False, sep=";")

    print(f"Day {args.day} - Total input: {tot_input}, Total output: {tot_output}")
    print(
        f"Day {args.day} - Total input synth: {tot_input_synth}, Total output synth: {tot_output_synth}"
    )
    print(f"Day {args.day} - Total inner: {tot_inner}")
    print(f"Day {args.day} - Total inner synth: {tot_inner_synth}")
    print(f"Total difference: {df_diff["sum"].sum(axis=0)}")

    plt.title("Real data - Synthetic data")
    plt.xlabel("Simulation time")
    plt.ylabel("Difference")
    plt.legend()
    plt.show()

    # plt.plot(mean_diff, label="Mean difference")
    # plt.show()

    df_data = pd.read_csv(f"./{args.day}/data.csv", sep=";")
    # if exists ./{args.day}-optimized/data.csv import it
    df_opt = None
    try:
        df_opt = pd.read_csv(f"./{args.day}-optimized/data.csv", sep=";")
    except FileNotFoundError:
        pass

    # plot mean_traveltime over mean_density for df_data
    plt.scatter(df_data["mean_density"], df_data["mean_traveltime"], label="Normal")
    if df_opt is not None:
        plt.scatter(
            df_opt["mean_density"], df_opt["mean_traveltime"], label="Optimized"
        )
    plt.xlabel("Mean density")
    plt.ylabel("Mean travel time")
    plt.legend()
    plt.show()
