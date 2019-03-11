import matplotlib.pyplot as plt

# 40 broadcasts

fd = open("eos_hgfr_e2e_tps_on_input.txt", 'r')
lines = fd.readlines()
x = []
y_eth = []
y_our = []
for line in lines[1:]:
    fields = line.split('\t')
    x.append(float(fields[0]))
    y_our.append(float(fields[1]))
    y_eth.append(float(fields[2]))

fig = plt.figure(figsize=(5,4))

ax = fig.add_subplot(111)
plt.plot(x, y_eth, marker='s', markersize=6, linewidth=2, label='100Mbps')
plt.plot(x, y_our, marker='s', markersize=6, linewidth=2, label='50Mbps')
ax.set_xticks(list(range(0,400, 100)))
ax.set_xticklabels(list(range(0,400, 100)), fontsize=12, weight="bold")
ax.set_yticks(list(range(0,250,50)))
ax.set_yticklabels(list(range(0,250,50)), fontsize=12, weight="bold")
ax.legend(fontsize=12, prop={'weight':'bold'}, loc='lower right')
plt.xlabel('Input transactions per second', fontsize=12, weight="bold")
plt.ylabel('Transactions per second', fontsize=12, weight="bold")

fig.tight_layout()
plt.savefig('eos_hgfr_e2e_tps_on_input.pdf')
plt.show()