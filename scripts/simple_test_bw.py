#!/usr/bin/env python
import argparse

from run_test_bw import *
from plot_test_bw import *

    
argParser = argparse.ArgumentParser()
argParser.add_argument("-l", "--logfile", default="client.log", help="logfile: client.log")
argParser.add_argument("-p", "--plotfile", default="bw.png", help="plotfile: bw.png")
argParser.add_argument("-n", "--size", default=1048576, help="size: 1048576")  
 
args = argParser.parse_args()

print("Running Simple Bandwidth Test With:")
print("Logging File:\t", args.logfile)
print("Plot File:\t", args.plotfile)
print("Buffer Size:\t", args.size)

run_test_bw([None, args.logfile], args.size)
plot_test_bw(args.logfile, args.plotfile)

