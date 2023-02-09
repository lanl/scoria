#!/usr/bin/env python
import argparse

from run_test_bw import *
from plot_test_bw import *

    
argParser = argparse.ArgumentParser()
argParser.add_argument("-l", "--logfile", default="client.log", help="logfile: (default) client.log")
argParser.add_argument("-p", "--plotfile", default="bw.png", help="plotfile: (default) bw.png")
argParser.add_argument("-n", "--size", default=1048576, help="size: (default) 1048576")  
argParser.add_argument("-s", "--bindscoria", default="", help="bind: hwloc-bind options (default) None")
argParser.add_argument("-b", "--bindclient", default="", help="bind: hwloc-bind options (default) None") 

args = argParser.parse_args()

print("Running Simple Bandwidth Test With:")
print("Logging File:\t", args.logfile)
print("Plot File:\t", args.plotfile)
print("Buffer Size:\t", args.size)
print("HWLOC Scoria Bind Options:\t", args.bindscoria)
print("HWLOC Client Bind Options:\t", args.bindclient)

run_test_bw([None, args.logfile], [args.bindscoria, args.bindclient], args.size)
plot_test_bw(args.logfile, args.plotfile)

