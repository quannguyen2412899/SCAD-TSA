'''
import os
import json
import graphviz

data = json.load (open('data/trie.json'))
labels = {i : data['labels'][i] for i in range(len(data['labels']))}
trie_root = data['root']
'''

import json
import graphviz 
import os
from typing import Dict, Any, List


NodeData = Dict[str, Any]

def visualize_trie_from_json_cleaned(json_file_path: str, output_filename: str = "trie_visualization_cleaned"):
    
    try:
        with open(json_file_path, 'r', encoding='utf-8') as f:
            data = json.load(f)
    except FileNotFoundError:
        print(f"Error: File not found at {json_file_path}")
        return
    except json.JSONDecodeError:
        print(f"Error: Invalid JSON format in file {json_file_path}")
        return

    # Extract core data
    totalUnique: int = data.get("totalUnique")
    anomalyThreshold: float = data.get("threshold")
    labels: List[str] = data.get('labels', [])
    root_node_data: NodeData = data.get('root', {})

    if not root_node_data:
        print("Error: 'root' field not found in JSON.")
        return

    # Initialize the directed graph
    dot = graphviz.Digraph('Trie', comment='Cleaned Trie Structure', graph_attr={'rankdir': 'LR'})

    def add_nodes_and_edges(node_data: NodeData, parent_id: str = None, edge_char: str = ""):

        current_id_int = node_data.get('ID')
        if current_id_int is None:
            return

        # Use ID as the unique identifier in Graphviz (but not in the displayed label)
        current_id = str(current_id_int)
        
        # Determine the character/label using the ID index
        node_char = labels[current_id_int] #if current_id_int < len(labels) else '?'
        
        # --- Define Node Properties ---
        count: int = node_data.get('count', 0)
        is_end = node_data.get('isEnd', False)
        is_anomaly = False #node_data.get('isAnomaly', False) # Assuming this field exists for visualization
        
        # Create the Label: Only character and count (excluding ID)

        children: Dict[str, NodeData] = node_data.get('children', {})

        if current_id_int == 0:
            label_display = "ROOT"
            shape = 'doublecircle'
        else:
            if is_end:
                for data in children.values():
                    count -= data.get('count', 0)
                label_display = f"'{node_char}'\n({count})"
                shape = 'circle'
            else:
                label_display = f"'{node_char}'"
                shape = 'box'

        # Set color based on anomaly status
        if (current_id_int != 0 and is_end):
            is_anomaly = (count / totalUnique) <= anomalyThreshold
        color = 'red' if is_anomaly else ('blue' if is_end else 'black')
        
        # Add Node
        dot.node(current_id, label_display, shape=shape, color=color, fontcolor=color)

        # --- Define Edge ---

        if parent_id is not None:
            # edge_char is the character that forms the key in the 'children' dictionary
            dot.edge(parent_id, current_id, label=edge_char)
        
        # --- Recursively process children ---
        
        for char_key, child_node_data in children.items():
            # char_key is the edge label
            add_nodes_and_edges(child_node_data, current_id, char_key)
            
    # Start the visualization process
    add_nodes_and_edges(root_node_data)

    # Render the file
    try:
        # Use cleanup=True to remove the intermediate .dot file
        dot.render(output_filename, view=True, format='png', cleanup=True)
        print(f"\nVisualization completed. File saved at: {output_filename}.png")
    except Exception as e:
        print(f"Error rendering the graph (Ensure Graphviz core tools are installed): {e}")


visualize_trie_from_json_cleaned('data/trie.json')