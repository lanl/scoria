#!/usr/bin/env python
from run_test_bw import *
from plot_test_bw import *

run_test_bw([None, 'client.log'])
plot_test_bw('client.log', 'bw.png')
