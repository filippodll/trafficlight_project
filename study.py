import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

day = "2024-06-06"
df_real = pd.read_csv(f"./june24/{day}.csv", sep=";")
df_synth = pd.read_csv(f"./{day}/out_spires.csv", sep=";")

COIL_DICT = { "0.127 4.45 6 1": 341, "0.127 4.46 6 1": 297, "4.46 0.127 6 1": 76, "4.46 4.45 4 1": 67, "4.45 4.46 8 1": 87, "0.127 4.47 6 1": 254, "0.127 4.46 2 1 ": 276, "4.45 4.44 4 1": 111, "4.46 4.47 8 1": 65, "0.127 4.44 2 1": 363, "4.45 4.46 8 1": 68, "4.41 4.42 4 1": 155, "4.44 4.41 4 1": 133, "4.41 4.33 6 1": 166, "0.127 4.41 6 1": 427, "2.6 2.10 6 1": 21, "4.47 4.46 4 1 ": 45, "4.44 4.45 8 1": 131, "2.5 2.6 2 1": 190, "4.47 2.6 8 1": 43, "2.6 4.47 4 1 ": 23, "2.10 2.6 6 1": 1, "4.42 4.41 8 1": 175, "0.127 4.44 6 1": 384, "2.6 2.5 6 1": 30, "0.127 4.47 2 1": 233, "4.41 4.44 8 1": 153}
OUTPUT_COILS = [21, 30, 76, 155, 166]

df_diff = pd.DataFrame()
df_diff["time"] = df_synth["time"]

for _, row in df_real.iterrows():
    idx = COIL_DICT.get(row["section"].strip())
    if not int(idx) in OUTPUT_COILS:
        continue
    data_real = row["data"].split("[")[1].split("]")[0]
    data_real = [int(d) for d in data_real.split(",")]
    data_synth = df_synth[str(idx)].to_list()

    # compute difference
    diff = np.subtract(data_real, data_synth)
    plt.plot(diff, label=f"Coil {idx // 21} -> {idx % 21}")
    df_diff[str(idx)] = diff

df_diff.to_csv(f"{day}-diff.csv", index=False, sep=";")

df_diff["time"] = pd.to_datetime(df_diff["time"])
df_diff = df_diff.set_index("time")

plt.xlabel("Simulation time")
plt.ylabel("Difference")
plt.legend()
plt.show()