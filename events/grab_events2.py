#! /usr/bin/python

x = open("sorted_events", "w")

with open("events.txt") as f:
	for line in f:
		parts = line.rstrip('\n' + '').split(" ")
		parts[:] = [item for item in parts if item != '']
		print(parts)

		if parts[2]=="Yes" and parts[3]==("No"):
			x.write(parts[0] + '\n')

