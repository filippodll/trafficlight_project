import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

day = "2023-05-25"
df_real = pd.read_csv(f"./may23/{day}.csv", sep=";")
df_synth = pd.read_csv(f"./{day}/out_spires.csv", sep=";")
df_data = pd.read_csv(f"./{day}/data.csv", sep=";")

COIL_DICT = { "0.127 4.45 6 1": 341, "0.127 4.46 6 1": 297, "4.46 0.127 6 1": 76, "4.46 4.45 4 1": 67, "4.45 4.46 8 1": 87, "0.127 4.47 6 1": 254, "0.127 4.46 2 1 ": 276, "4.45 4.44 4 1": 111, "4.46 4.47 8 1": 65, "0.127 4.44 2 1": 363, "4.45 4.46 8 1": 68, "4.41 4.42 4 1": 155, "4.44 4.41 4 1": 133, "4.41 4.33 6 1": 166, "0.127 4.41 6 1": 427, "2.6 2.10 6 1": 21, "4.47 4.46 4 1 ": 45, "4.44 4.45 8 1": 131, "2.5 2.6 2 1": 190, "4.47 2.6 8 1": 43, "2.6 4.47 4 1 ": 23, "2.10 2.6 6 1": 1, "4.42 4.41 8 1": 175, "0.127 4.44 6 1": 384, "2.6 2.5 6 1": 30, "0.127 4.47 2 1": 233, "4.41 4.44 8 1": 153}
OUTPUT_COILS = [21, 30, 76, 155, 166]
INPUT_COILS = [1, 175, 190, 211, 233, 254, 276, 297, 341, 363, 384, 427]

tot_input = 0
tot_output = 0
tot_input_synth = 0
tot_output_synth = 0

df_diff = pd.DataFrame()
df_diff["time"] = df_synth["time"]
df_in = pd.DataFrame()

for _, row in df_real.iterrows():
    idx = COIL_DICT.get(row["section"].strip())
    data_real = [int(d) for d in row["data"].split()]
    if int(idx) in INPUT_COILS:
        tot_input += sum(data_real)
        df_in[idx] = data_real
        tot_input_synth += df_synth[str(idx)].sum()
    elif int(idx) in OUTPUT_COILS:
        tot_output += sum(data_real)
        data_synth = df_synth[str(idx)].to_list()
        tot_output_synth += sum(data_synth)

        # compute difference
        if len(data_real) == len(data_synth):   
            diff = np.subtract(data_real, data_synth)
            plt.plot(diff, label=f"Coil {idx // 21} -> {idx % 21}")
            df_diff[str(idx)] = diff

print(f"Day {day} - Total input: {tot_input}, Total output: {tot_output}")
print(f"Day {day} - Total input synth: {tot_input_synth}, Total output synth: {tot_output_synth}")

df_diff.to_csv(f"{day}-diff.csv", index=False, sep=";")

df_diff = df_diff.set_index("time")

plt.title("Real data - Synthetic data")
plt.xlabel("Simulation time")
plt.ylabel("Difference")
plt.legend()
plt.show()

# plot mean_traveltime over mean_density for df_data
plt.scatter(df_data["mean_density"], df_data["mean_traveltime"], label="Real data")
plt.xlabel("Mean density")
plt.ylabel("Mean travel time")
plt.show()