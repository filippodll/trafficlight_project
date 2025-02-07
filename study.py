import argparse
import matplotlib.pyplot as plt

plt.style.use("tableau-colorblind10")
import numpy as np
import pandas as pd
from structures_out import COIL_DICT

# plt.rcParams['text.usetex'] = True


def alignYaxes(axes, align_values=None):
    """Align the ticks of multiple y axes

    Args:
        axes (list): list of axes objects whose yaxis ticks are to be aligned.
    Keyword Args:
        align_values (None or list/tuple): if not None, should be a list/tuple
            of floats with same length as <axes>. Values in <align_values>
            define where the corresponding axes should be aligned up. E.g.
            [0, 100, -22.5] means the 0 in axes[0], 100 in axes[1] and -22.5
            in axes[2] would be aligned up. If None, align (approximately)
            the lowest ticks in all axes.
    Returns:
        new_ticks (list): a list of new ticks for each axis in <axes>.

        A new sets of ticks are computed for each axis in <axes> but with equal
        length.
    """

    nax = len(axes)
    ticks = [aii.get_yticks() for aii in axes]
    if align_values is None:
        aligns = [ticks[ii][0] for ii in range(nax)]
    else:
        if len(align_values) != nax:
            raise Exception("Length of <axes> doesn't equal that of <align_values>.")
        aligns = align_values

    bounds = [aii.get_ylim() for aii in axes]
    for bound in bounds:
        if bound[0] < 0:
            bound = (0, bound[1])

    # align at some points
    ticks_align = [ticks[ii] - aligns[ii] for ii in range(nax)]

    # scale the range to 1-100
    ranges = [tii[-1] - tii[0] for tii in ticks]
    lgs = [-np.log10(rii) + 2.0 for rii in ranges]
    igs = [np.floor(ii) for ii in lgs]
    log_ticks = [ticks_align[ii] * (10.0 ** igs[ii]) for ii in range(nax)]

    # put all axes ticks into a single array, then compute new ticks for all
    comb_ticks = np.concatenate(log_ticks)
    comb_ticks.sort()
    locator = plt.MaxNLocator(nbins="auto", steps=[1, 2, 2.5, 3, 4, 5, 8, 10])
    new_ticks = locator.tick_values(comb_ticks[0], comb_ticks[-1])
    new_ticks = [new_ticks / 10.0 ** igs[ii] for ii in range(nax)]
    new_ticks = [new_ticks[ii] + aligns[ii] for ii in range(nax)]

    # find the lower bound
    idx_l = 0
    for i in range(len(new_ticks[0])):
        if any([new_ticks[jj][i] > bounds[jj][0] for jj in range(nax)]):
            idx_l = i - 1
            break

    # find the upper bound
    idx_r = 0
    for i in range(len(new_ticks[0])):
        if all([new_ticks[jj][i] > bounds[jj][1] for jj in range(nax)]):
            idx_r = i
            break

    # trim tick lists by bounds
    new_ticks = [tii[idx_l : idx_r + 1] for tii in new_ticks]

    # set ticks for each axis
    for axii, tii in zip(axes, new_ticks):
        axii.set_yticks(tii)

    return new_ticks


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--day", type=str, required=True, help="Day to study in the format YYYY-MM-DD"
    )
    parser.add_argument(
        "--n-nodes", type=int, required=True, help="Number of nodes in the network"
    )
    parser.add_argument("--input-folder", type=str, required=True, help="Input folder")
    parser.add_argument("--print-coildiff", action="store_true", help="Print coil diff")
    parser.add_argument(
        "--start-time", type=int, default=0, help="Start time"
    )  # in hours
    args = parser.parse_args()

    args.start_time = args.start_time * 12

    df_real = pd.read_csv(f"{args.input_folder}/{args.day}.csv", sep=";")
    df_synth = pd.read_csv(f"./output/{args.day}/output_counts.csv", sep=";")

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
    df_validation["time"] = np.convolve(
        df_synth["time"].to_list(), np.ones((12,)) / 12, mode="full"
    )
    df_validation["input"] = 0
    df_validation["output"] = 0

    fig_diff, ax_diff = plt.subplots(figsize=(16, 9))
    fig, ax = plt.subplots(figsize=(16, 9))

    # x labels: one point every 5 minutes from 8:00 to 20:00
    ref_time = np.arange(8 * 12, 20 * 12)
    # convert into time
    ref_time = [f"{int(t // 12):02d}:{int(t % 12) * 5:02d}" for t in ref_time]

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
                if int(idx) != 155:
                    df_diff[str(idx)] = diff
                # diff = diff * 65 / 85
                if int(idx) == 155:
                    ax.plot(
                        ref_time,
                        diff,
                        label=f"Coil {idx // args.n_nodes} -> {idx % args.n_nodes}",
                    )
                else:
                    ax_diff.plot(
                        ref_time,
                        diff,
                        label=f"Coil {idx // args.n_nodes} -> {idx % args.n_nodes}",
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

    ax_diff.set_title(
        f"{args.day}\nDifference between output REAL and SIMULATED data - hourly average",
        fontsize="xx-large",
    )
    ax_diff.set_xticks(np.arange(0, len(df_diff), 6))
    ax_diff.set_xticklabels(
        [
            f"{int(t // 12)+8:02d}:{int(t % 12) * 5:02d}"
            for t in np.arange(0, len(df_diff), 6)
        ],
        rotation=45,
    )
    ax_diff.grid(linestyle="--")
    ax_diff.set_xlabel("Simulation time", fontsize="xx-large")
    ax_diff.set_ylabel("Relative error (%)", fontsize="xx-large")
    ax_diff.legend(fontsize="xx-large")
    ax_diff.tick_params(axis="both", which="major", labelsize=14)
    fig_diff.savefig(f"./output/{args.day}/diff.png")

    ax.set_title(
        f"{args.day}\nDifference between output REAL and SIMULATED data - hourly average"
    )
    ax.set_xticks(np.arange(0, len(df_diff), 12))
    ax.set_xticklabels(
        [
            f"{int(t // 12)+8:02d}:{int(t % 12) * 5:02d}"
            for t in np.arange(0, len(df_diff), 12)
        ],
        rotation=45,
    )
    ax.grid(linestyle="--")
    ax.set_xlabel("Simulation time", fontsize="xx-large")
    ax.set_ylabel("Relative error (%)", fontsize="xx-large")
    ax.legend(fontsize="xx-large")
    ax.tick_params(axis="both", which="major", labelsize=14)
    fig.savefig(f"./output/{args.day}/wrong.png")

    df_data = pd.read_csv(f"./output/{args.day}/data.csv", sep=";")
    df_data["time"] = df_data["time"] // 300  # Each point is 5 minutes
    df_data["mean_density"] = df_data["mean_density"] * 1000  # convert to veh/km

    df_in["time"] = df_data["time"]
    df_in = df_in.set_index("time")
    df_in["total"] = df_in.sum(axis=1)

    # if exists ./{args.day}-optimized/data.csv import it
    df_opt_single = None
    df_opt_double = None
    try:
        df_opt_single = pd.read_csv(f"./output/{args.day}-single/data.csv", sep=";")
        df_opt_single["time"] = df_opt_single["time"] // 300
        df_opt_single["mean_density"] = df_opt_single["mean_density"] * 1000
    except FileNotFoundError:
        print(f"No optimized data found for {args.day}")
    try:
        df_opt_double = pd.read_csv(f"./output/{args.day}-double/data.csv", sep=";")
        df_opt_double["time"] = df_opt_double["time"] // 300
        df_opt_double["mean_density"] = df_opt_double["mean_density"] * 1000
    except FileNotFoundError:
        print(f"No optimized data found for {args.day}")

    if args.start_time > 0:
        df_data = df_data[args.start_time :]
        df_in = df_in[args.start_time :]
        if df_opt_single is not None:
            df_opt_single = df_opt_single[args.start_time :]
        if df_opt_double is not None:
            df_opt_double = df_opt_double[args.start_time :]

    ########################################################################################
    # Plot the mean travel time over the mean density
    ########################################################################################
    plt.figure(figsize=(16, 9))
    if df_opt_single is not None:
        plt.plot(
            df_opt_single["mean_density"],
            df_opt_single["mean_traveltime"],
            label="Single-tail optimization",
            marker="x",
        )
    if df_opt_double is not None:
        plt.plot(
            df_opt_double["mean_density"],
            df_opt_double["mean_traveltime"],
            label="Double-tail optimization",
            marker="^",
        )
    plt.plot(
        df_data["mean_density"], df_data["mean_traveltime"], label="Normal", marker="o"
    )
    plt.xlabel(r"Mean density $(veh/km)$", fontsize="xx-large")
    plt.ylabel(r"Mean travel time $(s)$", fontsize="xx-large")
    plt.grid(linestyle="--")
    plt.legend(fontsize="xx-large")
    plt.tick_params(axis="both", which="major", labelsize=14)
    plt.title(f"{args.day}\nMean travel time over mean density", fontsize="xx-large")
    plt.savefig(f"./output/{args.day}/traveltime_density.png")

    ########################################################################################
    # Plot the mean travel time over time
    ########################################################################################
    fig, ax = plt.subplots(figsize=(16, 9))
    print(f"Day {args.day}")
    print(
        f"Mean travel time: {int(df_data["mean_traveltime"].mean())} ± {int(df_data["mean_traveltime"].std())} s"
    )
    if df_opt_single is not None:
        ax.plot(
            df_opt_single["mean_traveltime"],
            label="Single-tail optimization",
        )
        print(
            f"Mean traveltime (single-tail opt): {int(df_opt_single["mean_traveltime"].mean())} ± {int(df_opt_single["mean_traveltime"].std())} s"
        )
    if df_opt_double is not None:
        ax.plot(
            df_opt_double["mean_traveltime"],
            label="Double-tail optimization",
        )
        print(
            f"Mean traveltime (double-tail opt): {int(df_opt_double["mean_traveltime"].mean())} ± {int(df_opt_double["mean_traveltime"].std())} s"
        )
    ax.plot(df_data["mean_traveltime"], label="Normal")
    ax.set_xticks(
        np.arange(args.start_time, 288, 12),
        [
            f"{int(t // 12):02d}:{int(t % 12) * 5:02d}"
            for t in np.arange(args.start_time, 288, 12)
        ],
        rotation=45,
        fontsize="xx-large",
    )
    ax.set_ylabel(r"Mean travel time $(s)$", fontsize="xx-large")
    ax.grid(linestyle="--")
    ax.legend(fontsize="xx-large", loc="upper left")
    ax.tick_params(axis="both", which="major", labelsize=14)
    plt.title(f"{args.day}\nMean travel time over time", fontsize="xx-large")

    plt.savefig(f"./output/{args.day}/traveltime.png")

    ########################################################################################
    # Plot the mean density over time
    ########################################################################################
    fig, ax = plt.subplots(figsize=(16, 9))
    if df_opt_single is not None:
        ax.plot(
            df_opt_single["time"],
            df_opt_single["mean_density"],
            label="Single-tail optimization",
        )
    if df_opt_double is not None:
        ax.plot(
            df_opt_double["time"],
            df_opt_double["mean_density"],
            label="Double-tail optimization",
        )
    ax.plot(df_data["time"], df_data["mean_density"], label="Normal")
    ax.set_xticks(
        np.arange(args.start_time, 288, 12),
        [
            f"{int(t // 12):02d}:{int(t % 12) * 5:02d}"
            for t in np.arange(args.start_time, 288, 12)
        ],
        rotation=45,
    )
    ax2 = ax.twinx()
    ax2.plot(
        df_in["total"],
        label="Total input",
        color="black",
        linestyle="--",
    )
    # ax2.plot(
    #     df_data["nGhosts"],
    #     label="Ghost agents",
    #     linestyle="--",
    # )
    # ax2.plot(
    #     df_opt_single["nGhosts"],
    #     label="Ghost agents - single",
    #     linestyle="--",
    # )
    # ax2.plot(
    #     df_opt_double["nGhosts"],
    #     label="Ghost agents - double",
    #     linestyle="--",
    # )
    ax.set_ylabel(r"Mean density $(veh/km)$", fontsize="xx-large")
    ax.grid(linestyle="--")
    ax.legend(fontsize="xx-large", loc="upper left")
    ticks = alignYaxes([ax, ax2], [0, 0])
    ax2.set_ylabel(r"Total input $(veh)$", fontsize="xx-large")
    ax2.legend(fontsize="xx-large", loc="upper right")
    ax.set_ybound(-1)
    # set bound of ax2 proportional to ax baisn gon ticks
    ratio = (ticks[1][1] - ticks[1][0]) / (ticks[0][1] - ticks[0][0])
    ax2.set_ybound(-ratio)

    plt.tick_params(axis="both", which="major", labelsize=14)
    plt.title(f"{args.day}\nMean density over time", fontsize="xx-large")
    plt.savefig(f"./output/{args.day}/density_time.png")

    ########################################################################################
    # Plot the difference between optimized and non optimized input flows
    ########################################################################################
    if args.print_coildiff:
        df_synth_single = pd.read_csv(
            f"./output/{args.day}-single/out_spires.csv", sep=";"
        )
        df_synth_double = pd.read_csv(
            f"./output/{args.day}-double/out_spires.csv", sep=";"
        )

        # Loop through each coil value
        for coil in INPUT_COILS:
            # Check if the coil (column) exists in both datasets
            if (
                str(coil) in df_synth.columns
                and str(coil) in df_synth_single.columns
                and str(coil) in df_synth_double.columns
            ):
                # Compute the difference for each row
                diff_ns = df_synth[str(coil)] - df_synth_single[str(coil)]

                # Plot the differences
                plt.figure(figsize=(16, 9))
                plt.plot(
                    diff_ns, label=f"Difference in column {coil}", color="tab:blue"
                )
                plt.title(
                    f"{args.day}\nDifference between df_synth and df_synth_single for Coil {coil}"
                )
                plt.xlabel("Row Index")
                plt.ylabel("Difference")
                plt.legend()
                plt.grid(True)

                # Save the plot or show it
                plt.savefig(f"./output/{args.day}/coil_{coil}_difference_single.png")
                plt.close()

                # Plot the difference with the double
                diff_nd = df_synth[str(coil)] - df_synth_double[str(coil)]
                plt.figure(figsize=(16, 9))
                plt.plot(
                    diff_nd, label=f"Difference in column {coil}", color="tab:blue"
                )
                plt.title(
                    f"{args.day}\nDifference between df_synth and df_synth_double for Coil {coil}"
                )
                plt.xlabel("Row Index")
                plt.ylabel("Difference")
                plt.legend()
                plt.grid(True)

                # Save the plot or show it
                plt.savefig(f"./output/{args.day}/coil_{coil}_difference_double.png")
                plt.close()

            else:
                print(f"Column {coil} not found in one of the datasets.")
