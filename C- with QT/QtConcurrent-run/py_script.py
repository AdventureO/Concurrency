#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os


count = int(input("How many times should I run a program? Enter a number: "))
for i in range(count):
    os.system('/home/yaryna/AKS_main/AKS_main')

total_time = []
reading_time = []
counting_time = []
counter = 0


for line in open('/home/yaryna/Desktop/res_a.txt', 'r'):

    line = int(line.replace("\n","").split(": ")[1])
    if counter % 3 == 0:
        total_time.append(line)

    elif counter % 3 == 1:
        reading_time.append(line)

    else:
        counting_time.append(line)

    counter += 1


print("Minimum total time: {0}".format(min(total_time)))
print("Minimum reading time: {0}".format(min(reading_time)))
print("Minimum counting time: {0}".format(min(counting_time)))
