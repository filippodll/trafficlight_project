import networkx as nx
import numpy as np
import osmnx as ox
import pandas as pd

# Take the network form OSM

G = ox.graph_from_place('Bologna, Italy', network_type='drive')
nodes, edges = ox.graph_to_gdfs(G)

# Reset index to include osmid , u , v ecc

nodes = nodes.reset_index()
edges = edges.reset_index()

# Extract nodes

nodes['highway'] = nodes['highway'].fillna('road')  # Se 'highway' ha valori mancanti, impostalo come 'road'
df_nodes = nodes[['osmid', 'y', 'x', 'highway']]
df_nodes.columns = ['id', 'lat', 'lon', 'highway']
df_nodes.to_csv("osm_nodes.csv", sep=';', index=False)

#Extract edges

edges['highway'] = edges['highway'].fillna('road')  # Se 'highway' ha valori mancanti, imposta un valore predefinito
df_edges = edges[['u', 'v', 'length', 'oneway', 'lanes', 'highway', 'maxspeed', 'bridge']]  # 'u' e 'v' rappresentano i nodi source e target
df_edges.columns = ['sourceID', 'targetID', 'length', 'oneway', 'lanes', 'highway', 'maxspeed', 'bridge']
df_edges.to_csv("osm_edges.csv", sep=';', index=False)