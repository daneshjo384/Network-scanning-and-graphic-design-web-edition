import tkinter as tk
from tkinter import ttk, messagebox
import nmap
import json
import networkx as nx
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import threading


def scan_network():
    nm = nmap.PortScanner()
  
    nm.scan(hosts='192.168.1.0/24', arguments='-sn')  
    devices = []
    for host in nm.all_hosts():
        mac = nm[host]['addresses'].get('mac', 'N/A')
        devices.append({
            'ip': host,
            'mac': mac
        })
    return devices


def save_to_json(devices, filename='network_scan.json'):
    with open(filename, 'w') as f:
        json.dump(devices, f, indent=2)
    print(f'Data saved to {filename}')


def draw_topology(devices, parent_frame):
    G = nx.Graph()
    router_ip = '192.168.1.1' 
    G.add_node(router_ip, label='Router', color='red')

    for dev in devices:
        ip = dev['ip']
        G.add_node(ip, label=ip, color='lightblue')
        G.add_edge(router_ip, ip)

    pos = nx.spring_layout(G)
    plt.clf()
    colors = [G.nodes[node].get('color', 'lightblue') for node in G.nodes]
    nx.draw(G, pos, with_labels=True, node_color=colors, node_size=1500, font_size=8, font_weight='bold')
    plt.title("Network Topology")

    canvas = FigureCanvasTkAgg(plt.gcf(), master=parent_frame)
    canvas.draw()
    canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)


class NetworkScannerApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Network Topology Scanner")
        self.root.geometry("800x600")

        self.tree_frame = tk.Frame(root)
        self.tree_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        self.graph_frame = tk.Frame(root)
        self.graph_frame.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True)

        self.tree = ttk.Treeview(self.tree_frame, columns=("IP", "MAC"), show='headings')
        self.tree.heading("IP", text="IP Address")
        self.tree.heading("MAC", text="MAC Address")
        self.tree.pack(fill=tk.BOTH, expand=True)

        self.scan_button = tk.Button(self.tree_frame, text="Scan Network", command=self.start_scan)
        self.scan_button.pack(pady=5)

        self.save_button = tk.Button(self.tree_frame, text="Save to JSON", command=self.save_data)
        self.save_button.pack(pady=5)

        self.devices = []

    def start_scan(self):
        self.tree.delete(*self.tree.get_children())
        self.devices = scan_network()
        for dev in self.devices:
            self.tree.insert("", "end", values=(dev['ip'], dev['mac']))
        messagebox.showinfo("Scan Complete", f"Found {len(self.devices)} devices.")

    def save_data(self):
        if not self.devices:
            messagebox.showwarning("No Data", "Please scan the network first.")
            return
        save_to_json(self.devices)
        messagebox.showinfo("Saved", "Data saved to network_scan.json")

    def show_topology(self):
        draw_topology(self.devices, self.graph_frame)

    def update_topology(self):
     
        for widget in self.graph_frame.winfo_children():
            widget.destroy()
        self.show_topology()

def run_app():
    root = tk.Tk()
    app = NetworkScannerApp(root)

    def on_scan():
        app.start_scan()
        app.update_topology()

    app.scan_button.config(command=on_scan)

    root.mainloop()

if __name__ == "__main__":
    run_app()