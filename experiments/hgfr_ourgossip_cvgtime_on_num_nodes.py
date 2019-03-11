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
           #'D']
GROUPS = ['Gossip', 'HGFRR']

HANDLES = [ mlines.Line2D([], [], color='black', marker=marker, linestyle='None',
            markersize=6, label=label) for marker, label in zip(MARKERS, GROUPS ) ]
HANDLES += [ mpatches.Patch(color=color, label=label) for color, label in  zip(COLORS, LABELS) ]

x = {}
y_eth200 = {}
y_our200 = {}

for file in FILES:
    x[file] = []
    y_eth200[file] = []
    y_our200[file] = []

    fd = open(file, 'r')
    lines = fd.readlines()

    for line in lines[1:]:
        fields = line.split('\t')
        x[file].append(float(fields[0]))
        y_eth200[file].append(float(fields[2]))
        y_our200[file].append(float(fields[1]))


fig = plt.figure(figsize=(9,6))

ax = fig.add_subplot(111)

for file, label, color in zip(FILES, LABELS, COLORS):
    plt.plot(x[file], y_eth200[file], marker='s', markersize=6, linewidth=2, label='Gossip with ' + label, color=color)
    plt.plot(x[file], y_our200[file], marker='o', markersize=6, linewidth=2, label='HGFRR with ' + label, color=color)

# plt.yscale('log')

ax.set_xticks(list(range(2000,10000,2000)))
ax.set_xticklabels(list(range(2000,10000,2000)), fontsize=12, weight="bold")
ax.set_yscale('log')
ax.set_yticks([1000,2000,5000,10000,20000,50000,100000])
ax.set_yticklabels([1,2,5,10,20,50,100], fontsize=12, weight="bold")
ax.legend(handles=HANDLES, fontsize=12, prop={'weight':'bold'}, loc='upper left')
plt.xlabel('Number of nodes', fontsize=12, weight="bold")
plt.ylabel('Convergence time (s)', fontsize=12, weight="bold")

fig.tight_layout()
plt.savefig('hgfr_ourgossip_cvgtime_on_num_nodes.pdf')
plt.show()