import networkx as nx
import pandas as pd
import geopandas as gpd
import numpy as np
import contextily as ctx
from matplotlib import pyplot as plt
from structures_out import COIL_DICT, NAME_DICT

OUTPUT_COILS = [21, 30, 76, 155, 166]
INPUT_COILS = [1, 175, 190, 211, 233, 254, 276, 297, 341, 363, 384, 427]
INNER_COILS = [
    c for c in COIL_DICT.values() if c not in OUTPUT_COILS and c not in INPUT_COILS
]
COLORMAP = {"output": "green", "input": "red", "inner": "blue", "other": "black"}

adj = np.loadtxt("./constants/adj.dat", skiprows=1)
coord = pd.read_csv("./constants/coords.csv", sep=";")
coord = coord.set_index("nodeId")

GDF = gpd.GeoDataFrame(
    coord, geometry=gpd.points_from_xy(coord.lon, coord.lat), crs="EPSG:4326"
)

G = nx.from_numpy_array(adj, create_using=nx.DiGraph)
edges = G.edges()
pos = {}
# coord has id as index with lat, lon columns
for node in G.nodes():
    pos[node] = (coord.loc[node]["lon"], coord.loc[node]["lat"])

for u, v in G.edges():
    edge_name = (
        u * G.number_of_nodes() + v
    )  # Replace 'dimension' with the actual dimension
    G.edges[u, v]["id"] = edge_name
    G.edges[u, v]["name"] = NAME_DICT[edge_name]
    # set color to yellow for id in OUTPUT_COIS, blue in INPUT_COILS, red in INNER_COILS and black otherwise
    if edge_name in OUTPUT_COILS:
        G.edges[u, v]["color"] = COLORMAP["output"]
    elif edge_name in INPUT_COILS:
        G.edges[u, v]["color"] = COLORMAP["input"]
    elif edge_name in INNER_COILS:
        G.edges[u, v]["color"] = COLORMAP["inner"]
    else:
        G.edges[u, v]["color"] = COLORMAP["other"]

edge_colors = [G[u][v]["color"] for u, v in G.edges()]

for node in G.nodes():
    if node in range(1, 8):
        G.nodes[node]["color"] = "orange"
    else:
        G.nodes[node]["color"] = "grey"

node_colors = [G.nodes[node]["color"] for node in G.nodes()]

# figsize double as the default
_, ax = plt.subplots(figsize=(16, 9))
limits = GDF.total_bounds + np.array([-0.001, -0.001, 0.001, 0.001])
ax.set_xlim(limits[0], limits[2])
ax.set_ylim(limits[1], limits[3])

nx.draw_networkx_edges(
    G,
    pos,
    edgelist=edges,
    edge_color=edge_colors,
    ax=ax,
    connectionstyle="arc3,rad=0.05",
    arrowsize=10,
    arrowstyle="->",
    width=2.5,
)
nx.draw_networkx_nodes(G, pos, ax=ax, node_size=500, node_color=node_colors)
nx.draw_networkx_labels(G, pos, ax=ax, font_size=20)
ctx.add_basemap(
    ax, crs=GDF.crs.to_string(), source=ctx.providers.OpenStreetMap.Mapnik, alpha=0.5
)
plt.box(False)
# add a legend for line colors
plt.legend(
    [
        plt.Line2D([0], [0], color=COLORMAP["output"], lw=4),
        plt.Line2D([0], [0], color=COLORMAP["input"], lw=4),
        plt.Line2D([0], [0], color=COLORMAP["inner"], lw=4),
        plt.Line2D([0], [0], color=COLORMAP["other"], lw=4),
        plt.Line2D(
            [0], [0], marker="o", color="w", markerfacecolor="orange", markersize=10
        ),
        plt.Line2D(
            [0], [0], marker="o", color="w", markerfacecolor="grey", markersize=10
        ),
    ],
    [
        "Output coils",
        "Input coils",
        "Inner coils",
        "Other coils",
        "Traffic Lights",
        "Other nodes",
    ],
    loc="upper right",
    title="Coil types",
    # increase dimension by a lot
    title_fontsize="xx-large",
    fontsize="xx-large",
)
# remove white space around the plot
plt.tight_layout()
plt.savefig("network.png", dpi=300)
