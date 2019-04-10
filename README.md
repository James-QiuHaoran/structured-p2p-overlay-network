# Augmenting Blockchain System Efficiency and Security with the Intel SGX Facilitated P2P Overlay Network

Final Year Project \@ Department of Computer Science, HKU

Team Member: [Haoran Qiu](https://james-qiuhaoran.github.io/index.html), Tao Ji

Supervisor: [Dr. Heming Cui](https://i.cs.hku.hk/~heming/)

## Introduction

Blockchain is essentially a public decentralized ledger that securely records transactions between parties anonymously. The key component is to reach agreement among a group of nodes, i.e., consensus. However, based on current Peer-to-Peer network, the broadcast operation is inefficient. Increasing transaction rate leads to traffic congestion. 

Intel SGX is a technology which protects code and execution integrity from hardware perspective. It provides chances to modify the network protocol and store some routing information, in order to reduce as many messages as possible and meanwhile retain the properties required by the blockchain application on top of the network. 

This project targets this weakness of the P2P network under blockchain systems, designing and implementing a new protocol in the Peer-to-Peer network which achieves effciency and do not sacrifice any security or any other required properties.

## Implementation

C++ version implementation of the p2p network, see `libgfrp2p`.

## Miscellaneous

- Project website: https://i.cs.hku.hk/fyp/2018/fyp18006/
- Contact: jamesqiu@hku.hk
- Copyright: GPL License
