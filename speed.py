from matplotlib import pyplot as plt
import polars as pl

df_data = pl.read_csv("./output/2023-05-19/speeds.csv", separator=";")
df_single = pl.read_csv("./output/2023-05-19-single/speeds.csv", separator=";")
df_double = pl.read_csv("./output/2023-05-19-double/speeds.csv", separator=";")
df_stoc = pl.read_csv("./output/2023-05-19-stoc/speeds.csv", separator=";")
# convert second column to list using split(,)
df_data = df_data.with_columns(
    pl.col("speeds").str.split(",").map_elements(lambda x: [float(i) for i in x[:-1]], return_dtype=pl.List(pl.Float64)).alias("speeds")
)
df_single = df_single.with_columns(
    pl.col("speeds").str.split(",").map_elements(lambda x: [float(i) for i in x[:-1]], return_dtype=pl.List(pl.Float64)).alias("speeds")
)
df_double = df_double.with_columns(
    pl.col("speeds").str.split(",").map_elements(lambda x: [float(i) for i in x[:-1]], return_dtype=pl.List(pl.Float64)).alias("speeds")
)
df_stoc = df_stoc.with_columns(
    pl.col("speeds").str.split(",").map_elements(lambda x: [float(i) for i in x[:-1]], return_dtype=pl.List(pl.Float64)).alias("speeds")
)
# print shape of dataframes

idx = int(18*12)


fig, ax = plt.subplots(figsize=(16, 9))
ax.hist(df_data[idx]["speeds"], bins=25, alpha=0.5, label="Normal")
ax.hist(
    df_single[idx]["speeds"],
    bins=25,
    alpha=0.5,
    label="Single-tail optimization",
)
ax.hist(
    df_double[idx]["speeds"],
    bins=25,
    alpha=0.5,
    label="Double-tail optimization",
)
ax.hist(df_stoc[idx]["speeds"], bins=25, alpha=0.5, label="Stochastic")
ax.set_xlabel(r"Mean travel speed $(m/s)$", fontsize="xx-large")
ax.set_ylabel("Frequency", fontsize="xx-large")
ax.set_yscale("log")
ax.legend(fontsize="xx-large")
ax.grid(linestyle="--")
plt.show()