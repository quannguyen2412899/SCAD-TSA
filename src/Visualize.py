import json
import graphviz 
import sys
import os
from typing import Dict, Any, List


NodeData = Dict[str, Any]

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
    totalUnique: int = data.get("totalUnique")
    anomalyThreshold: float = data.get("threshold")
    labels: List[str] = data.get('labels', [])
    root_data: NodeData = data.get('root', {})

    if not root_data:
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
        is_end = node_data.get('isEnd', False)
        children: Dict[str, NodeData] = node_data.get('children', {})

        if is_end:
            count: int = node_data.get('count', 0)
            for data in children.values():
                count -= data.get('count', 0)
            label_display = f"'{node_char}'\n({count})"
            if count/totalUnique <= anomalyThreshold:
                color = 'red'   # is anomaly
            else:
                color = 'green'
        else:
            label_display = node_char
            color = 'black'

        # Add Node
        dot.node(current_id, label_display, shape='circle', color=color, fontcolor=color)

        # --- Define Edge ---
        if parent_id is not None:
            # edge_char is the character that forms the key in the 'children' dictionary
            dot.edge(parent_id, current_id, label=node_char)
        
        # --- Recursively process children ---
        for char_key, child_node_data in children.items():
            # char_key is the edge label
            add_nodes_and_edges(child_node_data, current_id, char_key)
            
    # Start the visualization process
    add_nodes_and_edges(root_data)

    # Render the file
    try:
        # Use cleanup=True to remove the intermediate .dot file
        dot.render(output_path, view=True, format='png', cleanup=True)
        print(f"\nVisualization completed. File saved at: {output_path}.png")
    except Exception as e:
        print(f"Error rendering the graph (Ensure Graphviz core tools are installed): {e}")



def main():
    if len(sys.argv) == 1:
        input_file = 'data/interim/trie.json'
        output_file = 'data/output/trie_visualization'
    else:
        input_file = sys.argv[1]
        output_file = sys.argv[2]
    visualize_json_trie(input_file, output_file)


if __name__ == '__main__':
    main()