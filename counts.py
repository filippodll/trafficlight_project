import numpy as np
import networkx as nx
import pandas as pd

from structures_out import COIL_DICT, NAME_DICT

OUTPUT_COILS = [21, 30, 76, 155, 166]
INPUT_COILS = [1, 175, 190, 211, 233, 254, 276, 297, 341, 363, 384, 427]
INNER_COILS = [
    c for c in COIL_DICT.values() if c not in OUTPUT_COILS and c not in INPUT_COILS
]

DAY = "2023-05-09"

# read from a txt the adjacency matrix, skipping first line
A = np.loadtxt("constants/adj.dat", skiprows=1)

# create a graph from the adjacency matrix
G = nx.from_numpy_array(A, create_using=nx.DiGraph())

# assign edge names like "srcnode*dimension+dstnode" as integer
# Iterate over edges and assign names
for u, v in G.edges():
    edge_name = (
        u * G.number_of_nodes() + v
    )  # Replace 'dimension' with the actual dimension
    G.edges[u, v]["id"] = edge_name
    G.edges[u, v]["name"] = NAME_DICT[edge_name]

# # print graph edges
# print(G.edges(data=True))

# get a dict with the edges and their names
edges = nx.get_edge_attributes(G, "name")
# replace the keys with the ids
edges = {G.edges[e]["id"]: v for e, v in edges.items()}
# print(edges)

# assert n nodes is 21
assert G.number_of_nodes() == 21
# assert n edges is 35
assert G.number_of_edges() == 35

df = pd.read_csv(f"./data/{DAY}.csv", sep=";")
df["data"] = df["data"].apply(lambda x: [int(i) for i in x.split()])
# transform it ito numpy arrays
df["data"] = df["data"].apply(np.array)
# of the array keep only data from 8*12 to 20*12
# df["data"] = df["data"].apply(lambda x: x[8 * 12:20 * 12])
# for each entry in section, find where edges has this entry as value and replace it with the key
# Get a dictionary of edge names to IDs
edge_name_to_id = {v: k for k, v in edges.items()}

# Replace edge names with IDs in the DataFrame
df["section"] = df["section"].map(edge_name_to_id)
# print(df)

# for each node, compute the difference between input and output
diff_list = []
for i in range(G.number_of_nodes()):
    # get the input and output edges
    input_edges = list(G.in_edges(i))
    output_edges = list(G.out_edges(i))
    # get the input and output values
    input_values = df[df["section"].isin([G.edges[e]["id"] for e in input_edges])][
        "data"
    ].values
    output_values = df[df["section"].isin([G.edges[e]["id"] for e in output_edges])][
        "data"
    ].values

    out_sum = output_values.sum()
    in_sum = input_values.sum()
    # compute the difference
    diff = out_sum - in_sum
    diff_list.append(diff)
    # Reduce dimensions
    while isinstance(out_sum, np.ndarray):
        out_sum = sum(out_sum)
    while isinstance(in_sum, np.ndarray):
        in_sum = sum(in_sum)
    print(f"Node {i} in: {in_sum} out: {out_sum}")

print(f"\n\n\nDAY: {DAY}")
max_flow = int(25 / 115 * 300)
print(f"Max flow supported by 19 -> 7: {max_flow} veh/5min")
print(f"Missing data in 19 -> 7: {diff_list[8]}")
# check how many times diff_list[8] is greater than max_flow
n_times_over = sum([1 for i in diff_list[8] if i > max_flow])
print(
    f"Node 19 -> 7 would exceed max flow {n_times_over} times which corresponds to {n_times_over*5} minutes\n\n\n"
)
