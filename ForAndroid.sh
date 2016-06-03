#!/bin/bash

sudo systemctl stop firewalld
sudo service mysqld start
./server 192.168.2.100 10079
