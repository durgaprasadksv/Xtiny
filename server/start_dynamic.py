#!/usr/bin/python 

import os
import sys

def main(count):
	for i in range(0, count):
		os.system('nohup ./snapy /tmp/' + str(i) + '.sock &')

if __name__ == '__main__':
	main(int(sys.argv[1]))