#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib import cycler

colors = cycler('color',
                ['#EE6666', '#3388BB', '#9988DD',
                 '#EECC55', '#88BB44', '#FFBBBB'])
plt.rcParams["figure.figsize"] = (10,8)
plt.rc('axes', facecolor='#E6E6E6', edgecolor='none',
       axisbelow=True, grid=True, prop_cycle=colors)
plt.rc('grid', color='w', linestyle='solid')
plt.rc('xtick', direction='out', color='gray')
plt.rc('ytick', direction='out', color='gray')
plt.rc('patch', edgecolor='#E6E6E6')
plt.rc('lines', linewidth=2)

def ezplot(data, sp=[], width=3, height=3):
    df=pd.read_csv(data, skip_blank_lines=True, skipinitialspace=True, header=0, comment='#')
    df['speedup'] = df[df['threads']==1]['time'].min() / df['time']
    df['eff'] = df['speedup'] / df['threads']
    df=df.set_index('threads')
    multi=len(sp)>0
    if multi:
        g=df.groupby(sp)
        cols=g.ngroups
    else:
        cols=1
    fig, ax = plt.subplots(3, cols, sharex=True, sharey='row', figsize=(width*cols,height*3))
    ylab = ['temps', 'accélération', 'efficacité']
    for j,jcol in enumerate(['time', 'speedup', 'eff']):
        if multi:
            for i, (bs, df) in enumerate(g):
                ax[j][i].set_ylabel(ylab[j])
                df.groupby([ c for c in df.columns if c not in [*sp,'threads','time','speedup','eff']])[jcol].plot(legend=True,marker='o',ax=ax[j][i],title=bs if j==0 else None)
                ax[j][i].legend().remove()
        else:
            ax[j].set_ylabel(ylab[j])
            df.groupby([ c for c in df.columns if c not in [*sp,'threads','time','speedup','eff']])[jcol].plot(legend=True,marker='o',ax=ax[j])
            ax[j].legend().remove()
    if multi: fig.suptitle(*sp)
    plt.legend(*(ax[0][0] if multi else ax[0]).get_legend_handles_labels())
    plt.show()

if __name__ == '__main__':
    from sys import argv
    ezplot(argv[1],['bs'])
    ezplot(argv[1],['schedule'],width=4,height=4)
