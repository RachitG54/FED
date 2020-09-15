#!/bin/bash

sudo tc qdisc del dev lo root
sudo tc qdisc add dev lo root handle 1: htb default 12
sudo tc class add dev lo parent 1:1 classid 1:12 htb rate 620mbps ceil 620mbps
sudo tc qdisc add dev lo parent 1:12 netem delay 0.1ms 0.01ms distribution normal
sudo tc -s qdisc ls dev lo 
