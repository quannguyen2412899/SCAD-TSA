import json
import graphviz 
import sys
import os
from typing import Dict, Any, List

NodeData = Dict[str, Any]


totalUnique: int
anomalyThreshold: float
labels: List[str]
root_data: NodeData

# Initialize the directed graph
dot = graphviz.Digraph('Trie', comment='Cleaned Trie Structure', graph_attr={'rankdir': 'LR'})

def add_nodes_and_edges(node_data: NodeData, parent_id: str = None, edge_char: str = ""):

    current_id_int = node_data.get('id')
    if current_id_int is None:
        return

    # Get id, label, count, is_end
    current_id = str(current_id_int)
    node_char: str = node_data.get('label', "")
    count: int = node_data.get('count', 0)
    is_end = node_data.get('isEnd', False)
    children: Dict[str, NodeData] = node_data.get('children', {})

    # Display config
    label_display = f"{node_char}" + (" *" if is_end else "") + f"\n({count})"
    color: str = node_data.get('color', 'black')    
    if color == 'red':
        fillcolor = 'tomato'
    else:
        fillcolor = 'white'

    # Add node
    dot.node(current_id, label_display, style = 'filled', fillcolor = fillcolor, shape='circle', color=color, fontcolor='black')
    # Add edge
    if parent_id is not None:
        dot.edge(parent_id, current_id, label=edge_char)
    
    # Recursively draw subtree
    for char_key, child_node_data in children.items():
        add_nodes_and_edges(child_node_data, current_id, char_key)


def visualize_json_trie(input_path: str, output_path: str):
    try:
        with open(input_path, 'r', encoding='utf-8') as f:
            data = json.load(f)
    except FileNotFoundError:
        print(f"Error: File not found at {input_path}")
        return
    except json.JSONDecodeError:
        print(f"Error: Invalid JSON format in file {input_path}")
        return
    
    # Extract core data
    root_data: NodeData = data.get('root', {})

    if not root_data:
        print("Error: 'root' field not found in JSON.")
        return
    
    # Start the visualization process
    add_nodes_and_edges(root_data)

    # Render the file
    try:
        # Use cleanup=True to remove the intermediate .dot file
        dot.render(output_path, view=True, format='png', cleanup=True)
        print(f"\nVisualization completed. File saved at: {output_path}.png")
    except Exception as e:
        print(f"Error rendering the graph (Ensure Graphviz core tools are installed): {e}")



if __name__ == "__main__":
    root, old_ext = os.path.splitext(sys.argv[1])
    visualize_json_trie(sys.argv[1], root)
