import pandas as pd

# read output/2023-05-19.scv
df = pd.read_csv('output/2023-05-05/data.csv', sep=';')

df_stoc = pd.read_csv('output/2023-05-05-stoc/data.csv', sep=';')

# df_notl = pd.read_csv('output/2023-05-19-notl/data.csv', sep=';')

# plot mean_density over time for both

import matplotlib.pyplot as plt

# plt.plot(df['time'], df['mean_flow'], label='mean_density')
# plt.plot(df_stoc['time'], df_stoc['mean_flow'], label='mean_density_stoc')
# # plt.plot(df_notl['time'], df_notl['mean_flow'], label='mean_density_notl')
# plt.legend()

# plt.show()

plt.plot(df['time'], df['mean_density'], label='Realistic traffic lights')
plt.plot(df_stoc['time'], df_stoc['mean_density'], label='Passage probability')
# plt.plot(df_notl['time'], df_notl['mean_density'], label='mean_density_notl')
# Labels and legend with bigger fonts
plt.xlabel('Time (s)', fontsize=18)
plt.ylabel('Mean density (a.u.)', fontsize=18)
plt.legend(fontsize=16)

# Increase tick size
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)

plt.show()