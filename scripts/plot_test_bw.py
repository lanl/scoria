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
        
        not_started = True
        next_is_header = False

        for line in f:
            if not_started:
                if "Running tests" in line:
                    not_started = False
                    next_is_header = True
                    titles.append(line)
            else:
                if "Running tests" in line:
                    if not df.empty:
                        dfs.append(df)
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
                else:
                    line = line.split()

                    if len(line) > 0:
                        line = [line[i] for i in range(0, len(line)) if (i + 1) % 3 != 0]
                        line = line[:len(df.columns)]
                        line = [float(val) for  val in line]

                        df.loc[len(df.index)] = line
        dfs.append(df)

    for df, title in zip(dfs, titles):
        fig, axes = plt.subplots(nrows=2, ncols=1, figsize=(16, 10))

        dfr = df.filter(regex='(R)|Threads')
        dfr = dfr.set_index('Threads')
        dfr.plot.bar(width=0.5, ax=axes[0])
        axes[0].set_title(title + "(Read)")
        axes[0].set_xlabel('# Threads')
        axes[0].set_ylabel("Bandwidth (GiB/s)")
        axes[0].legend(ncol=3)

        dfw = df.filter(regex='(W)|Threads')
        dfw = dfw.set_index('Threads')
        dfw.plot.bar(width=0.5, ax=axes[1])
        axes[1].set_title(title + "(Write)")
        axes[1].set_xlabel('# Threads')
        axes[1].set_ylabel("Bandwidth (GiB/s)")
        axes[1].legend(ncol=3)

        plt.tight_layout()

        title = ''.join(title.split()[3:])
        figname = title.replace(' ', '_').strip() + '_' + out_file
        plt.savefig(figname)
