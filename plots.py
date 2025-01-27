import pandas as pd

# read output/2023-05-19.scv
df = pd.read_csv('output/2023-05-19/data.csv', sep=';')

df_stoc = pd.read_csv('output/2023-05-19-stoc/data.csv', sep=';')

df_notl = pd.read_csv('output/2023-05-19-notl/data.csv', sep=';')

# plot mean_density over time for both

import matplotlib.pyplot as plt

plt.plot(df['time'], df['mean_flow'], label='mean_density')
plt.plot(df_stoc['time'], df_stoc['mean_flow'], label='mean_density_stoc')
plt.plot(df_notl['time'], df_notl['mean_flow'], label='mean_density_notl')
plt.legend()

plt.show()

plt.plot(df['time'], df['mean_density'], label='mean_density')
plt.plot(df_stoc['time'], df_stoc['mean_density'], label='mean_density_stoc')
plt.plot(df_notl['time'], df_notl['mean_density'], label='mean_density_notl')
plt.legend()

plt.show()