import matplotlib.pyplot as plt
import matplotlib.lines as mlines
import matplotlib.patches as mpatches


FILES = ['hgfr_ourgossip_50cvgtime_on_num_nodes.txt',
         'hgfr_ourgossip_67cvgtime_on_num_nodes.txt',
         'hgfr_ourgossip_83cvgtime_on_num_nodes.txt',
         'hgfr_ourgossip_95cvgtime_on_num_nodes.txt']

LABELS = ['1/2 convergence',
          '2/3 convergence',
          '5/6 convergence',
          '95% convergence']

COLORS = [
    '#1f77b4',
    '#ff7f0e',
    '#2ca02c',
    '#d62728',
]

MARKERS = ['s',
           #'X',
           'o']
           #'d']

GROUPE = ['Gossip', 'HGFRR']

HANDLES = [ mlines.Line2D([], [], color='black', marker=marker, linestyle='None',
            markersize=6, label=label) for marker, label in zip(MARKERS, GROUPE) ]
HANDLES += [ mpatches.Patch(color=color, label=label) for color, label in zip(COLORS, LABELS) ]


x = [200,800,1250,1600,2000]

y_our2k = {}
y_eth2k = {}
y_our6k = {}
y_eth6k = {}
    

for file in FILES:
    fd = open(file, 'r')
    lines = fd.readlines()
    y_our2k[file] = []
    y_eth2k[file] = []
    y_our6k[file] = []
    y_eth6k[file] = []
    
    mtx = [ line.split('\t')[1:] for line in lines[1:] ]
    for i in range(0,10,2):
        y_our2k[file].append(float(mtx[1][i]))
        y_eth2k[file].append(float(mtx[1][i+1]))
        y_our6k[file].append(float(mtx[3][i]))
        y_eth6k[file].append(float(mtx[3][i+1]))
                    

fig = plt.figure(figsize=(7, 5.5))

ax = fig.add_subplot(111)
for file, color in zip(FILES, COLORS):
    # plt.plot(x, y_eth6k[file], marker='s', markersize=6, linewidth=2, label='Gossip with 6K nodes', color=color)
    # plt.plot(x, y_our6k[file], marker='X', markersize=6, linewidth=2, label='HGFRR with 6K nodes', color=color)
    plt.plot(x, y_eth2k[file], marker='s', markersize=6, linewidth=2, label='Gossip with 2K nodes', color=color)
    plt.plot(x, y_our2k[file], marker='o', markersize=6, linewidth=2, label='HGFRR with 2K nodes', color=color)

ax.set_xticks([200, 800, 1250, 1600, 2000])
ax.set_xticklabels([200, 800, 1250, 1600, 2000], fontsize=12, weight="bold")
# ax.set_yticks(list(range(2000,27000,2000)))
# ax.set_yticklabels(list(range(2,27,2)), fontsize=12, weight="bold")
ax.set_yscale('log')
ax.set_yticks([1000,2000,5000,10000,20000,50000,100000])
ax.set_yticklabels([1,2,5,10,20,50,100], fontsize=12, weight="bold")

lgd=ax.legend(handles=HANDLES, fontsize=12, prop={'weight':'bold'}, loc=9, bbox_to_anchor=(0.45, -0.1), ncol=3)
plt.xlabel('Number of broadcast operations per second', fontsize=12, weight="bold")
plt.ylabel('Convergence time (s)', fontsize=12, weight="bold")
plt.subplots_adjust(bottom=0.18,top = 0.97, left= 0.1, right=0.97)
plt.savefig('hgfr_ourgossip_cvgtime_on_tps_2k.pdf')
plt.show()