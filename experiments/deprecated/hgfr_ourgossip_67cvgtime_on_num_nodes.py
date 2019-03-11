import matplotlib.pyplot as plt
import matplotlib

# 40 broadcasts

fd = open("hgfr_ourgossip_67cvgtime_on_num_nodes.txt", 'r')
lines = fd.readlines()
x = []
y_eth200 = []
y_eth2000 = []
y_our200 = [] 
y_our2000 = []

for line in lines[1:]:
    fields = line.split('\t')
    x.append(float(fields[0]))
    y_eth200.append(float(fields[2]))
    y_our200.append(float(fields[1]))
    y_eth2000.append(float(fields[10]))
    y_our2000.append(float(fields[9]))
    
fig = plt.figure(figsize=(5,4))

ax = fig.add_subplot(111)
# plt.plot(x, y_eth2000, marker='v', markersize=6, linewidth=2, label='Gossip with 2000 nodes', color='#1f77b4')
# plt.plot(x, y_our2000, marker='D', markersize=6, linewidth=2, label='HGFRR with 2000 nodes', color='#ff7f0e')
plt.plot(x, y_eth200, marker='o', markersize=6, linewidth=2, label='Gossip', color='#1f77b4')
plt.plot(x, y_our200, marker='v', markersize=6, linewidth=2, label='HGFRR', color='#ff7f0e')

# plt.yscale('log')

ax.set_xticks(list(range(2000,10000,2000)))
ax.set_xticklabels(list(range(2000,10000,2000)), fontsize=12, weight="bold")
ax.set_yscale('log')
ax.set_yticks([1000,2000,5000,10000,20000,50000,100000])
ax.set_yticklabels([1,2,5,10,20,50,100], fontsize=12, weight="bold")
# ax.get_yaxis().set_major_formatter(matplotlib.ticker.ScalarFormatter())
ax.legend(fontsize=12, prop={'weight':'bold'}, loc='upper left')
plt.xlabel('Number of nodes', fontsize=12, weight="bold")
plt.ylabel('Convergence time (s)', fontsize=12, weight="bold")

fig.tight_layout()
plt.savefig('hgfr_ourgossip_67cvgtime_on_num_nodes.pdf')
plt.show()