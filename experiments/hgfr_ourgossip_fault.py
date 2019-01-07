import matplotlib.pyplot as plt

# 40 broadcasts

fd = open("hgfr_ourgossip_fault.txt", 'r')
lines = fd.readlines()
x = []
y_eth = []
y_our = []
for line in lines[1:]:
    fields = line.split('\t')
    x.append(float(fields[0][:-1]))
    y_our.append(float(fields[2][:-2]))
    y_eth.append(float(fields[1][:-1]))

fig = plt.figure(figsize=(6,4))

ax = fig.add_subplot(111)
plt.plot(x, y_eth, marker='s', markersize=6, linewidth=2, label='Gossip')
plt.plot(x, y_our, marker='o', markersize=6, linewidth=2, label='HGFRR')
ax.set_xticks(list(range(0,100, 20)))
ax.set_xticklabels(list(range(0,100, 20)), fontsize=12, weight="bold")
ax.set_yticks(list(range(0,120, 20)))
ax.set_yticklabels(list(range(0,120, 20)), fontsize=12, weight="bold")
ax.legend(fontsize=12, prop={'weight':'bold'}, loc='lower left')
plt.xlabel('Percentage of failed nodes (%)', fontsize=12, weight="bold")
plt.ylabel('Percentage of living nodes reached (%)', fontsize=12, weight="bold")

fig.tight_layout()
plt.savefig('hgfr_ourgossip_fault.pdf')
plt.show()