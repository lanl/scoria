#!/usr/bin/env python

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

def plot_test_bw(in_file, out_file):
    print(in_file, out_file)

    titles = []
    dfs = []

    with open(in_file) as f:
        df = pd.DataFrame()
        next_is_header = False

        for _ in range(9):
            next(f)
        for line in f:
            # do stuff
            if "Running tests" in line:
                next_is_header = True

                titles.append(line)
            elif next_is_header:
                next_is_header = False
        
                line = line.split()
                tmp = line[0]
                line = np.repeat(line[1:], 2)
                line = [line[i] + '(R)' if i % 2 == 0 else line[i] + '(W)' for i in range(len(line))]
                line = np.concatenate((tmp, line), axis=None)
                
                df = pd.DataFrame(columns=line)
            elif line == '\n':
                dfs.append(df)
            else:
                line = line.split()

                line = [line[i] for i in range(0, len(line)) if (i + 1) % 3 != 0]
                line = line[:len(df.columns)]
                line = [float(val) for  val in line]

                df.loc[len(df.index)] = line

        dfs.append(df)

    for df, title in zip(dfs, titles):
        print(title, df)

        dfr = df.filter(regex='(R)|Threads')
        dfr.plot.bar()
        plt.show()

        dfw = df.filter(regex='(W)|Threads')
