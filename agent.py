import pandas as pd
from matplotlib import pyplot as plt

DAY = "2023-05-12"

df = pd.read_csv(f"./output/{DAY}/agent_dump.csv", sep=";")
# keep only rows where street column is empty
df = df[df["street"].isna()]

# make a histogram of src column
df["src"].hist()
plt.show()
