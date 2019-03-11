import matplotlib.pyplot as plt

# 40 broadcasts

fd = open("eth_ourgossip_num_msg_on_num_node.txt", 'r')
lines = fd.readlines()
x = []
y_eth = []
y_our = []
for line in lines[1:]:
    fields = line.split('\t')
    x.append(float(fields[0]))
    y_eth.append(float(fields[1][:-1]))
    y_our.append(float(fields[2][:-2]))

fig = plt.figure(figsize=(5, 4))

ax = fig.add_subplot(111)
plt.plot(x, y_eth, marker='o', markersize=6, linewidth=2, label='Ethereum')
plt.plot(x, y_our, marker='v', markersize=6, linewidth=2, label='Self-implemented')
ax.set_xticks(list(range(40,280, 40)))
ax.set_xticklabels(list(range(40,280,40)), fontsize=12, weight="bold")
ax.set_yticks(list(range(10,70,10)))
ax.set_yticklabels(list(range(10,70,10)), fontsize=12, weight="bold")
ax.legend(fontsize=12, prop={'weight':'bold'}, loc='lower right')
plt.xlabel('Number of nodes', fontsize=12, weight="bold")
plt.ylabel('Number of messages (K)', fontsize=12, weight="bold")

fig.tight_layout()
plt.savefig('eth_ourgossip_num_msg_on_num_node.pdf')
plt.show()