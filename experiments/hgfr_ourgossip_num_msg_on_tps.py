import matplotlib.pyplot as plt
import matplotlib.lines as mlines
import matplotlib.patches as mpatches

# 40 broadcasts

fd = open("hgfr_ourgossip_num_msg_on_num_node.txt", 'r')
lines = fd.readlines()
x = []
y_eth2k = []
y_eth6k = []
y_our2k = [] 
y_our6k = []

x = [200,800,1250,1600,2000]

line = lines[1]
fields = line.split('\t')
for i in range(1,11,2):
    y_our2k.append(float(fields[i]))
    y_eth2k.append(float(fields[i+1]))

line = lines[3]
fields = line.split('\t')
for i in range(1,11,2):
    y_our6k.append(float(fields[i]))
    y_eth6k.append(float(fields[i+1]))


fig = plt.figure(figsize=(5,4))

ax = fig.add_subplot(111)
plt.plot(x, y_eth6k, marker='s', markersize=6, linewidth=2, label='Gossip with 6K nodes', color='#1f77b4')
plt.plot(x, y_our6k, marker='o', markersize=6, linewidth=2, label='HGFRR with 6K nodes', color='#1f77b4')
plt.plot(x, y_eth2k, marker='s', markersize=6, linewidth=2, label='Gossip with 2K nodes', color='#ff7f0e')
plt.plot(x, y_our2k, marker='o', markersize=6, linewidth=2, label='HGFRR with 2K nodes', color='#ff7f0e')

HANDLES = [ mlines.Line2D([], [], color='black', marker=marker, linestyle='None',
            markersize=6, label=label) for marker, label in zip(['s', 'o'], ['Gossip', 'HGFRR']) ]
HANDLES += [ mpatches.Patch(color=color, label=label) for color, label in zip(['#1f77b4', '#ff7f0e'], ['6K nodes', '2K nodes']) ]


ax.set_xticks([200, 800, 1250, 1600, 2000])
ax.set_xticklabels([200, 800, 1250, 1600, 2000], fontsize=12, weight="bold")
ax.set_yticks(list(range(2000,27000,2000)))
ax.set_yticklabels(list(range(2,27,2)), fontsize=12, weight="bold")
ax.legend(handles=HANDLES, fontsize=12, prop={'weight':'bold'}, loc='upper right', ncol=2)
plt.xlabel('Number of broadcast operations per second', fontsize=12, weight="bold")
plt.ylabel('Number of messages (K)', fontsize=12, weight="bold")

fig.tight_layout()
plt.savefig('hgfr_ourgossip_num_msg_on_tps.pdf')
plt.show()