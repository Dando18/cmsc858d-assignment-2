''' Create results plots.
    Author: Daniel Nichols
    date: April 2022
'''
# std lib imports
import os
import sys

# tpl imports
import matplotlib.pyplot as plt
import pandas as pd


def generate_buildsa_plots(df_seq, df_par):
    ''' Create the buildsa experiment plots for part 1.
    '''

    fig, (ax1, ax2) = plt.subplots(1,2)
    
    # Sequential build time plots 
    for prefix in df_seq['preftab'].unique():
        cur_df = df_seq[df_seq['preftab'] == prefix]
        ax1.plot(cur_df['seq_length'], cur_df['build_time'], marker='^', lw=2, label='k='+str(prefix))
    
    ax1.set_title('Sequential')
    ax1.set_xlabel('# Base Pairs')
    ax1.set_ylabel('Time (ms)')
    ax1.set_xscale('log', base=10)
    ax1.legend()

    # Parallel build time plots 
    for prefix in df_par['preftab'].unique():
        cur_df = df_par[df_par['preftab'] == prefix]
        ax2.plot(cur_df['seq_length'], cur_df['build_time'], marker='^', lw=2, label='k='+str(prefix))
    
    ax2.set_title('Multi-Threaded')
    ax2.set_xlabel('# Base Pairs')
    ax2.set_xscale('log', base=10)
    ax2.get_yaxis().set_visible(False)
    ax2.set_ylim(ax1.get_ylim())
    fig.suptitle('Sequence Length vs Build Time')
    fig.savefig(os.path.join('figs', 'seq_length_vs_build_time.png'))

    # File size plots
    plt.clf()
    for prefix in df_seq['preftab'].unique():
        cur_df = df_seq[df_seq['preftab'] == prefix]
        plt.plot(cur_df['seq_length'], cur_df['file_size'] / 1E6, marker='^', lw=2, label='k='+str(prefix))

    plt.title('Sequence Length vs File Size')
    plt.xlabel('# Base Pairs')
    plt.ylabel('File Size (MB)')
    plt.xscale('log', base=10)
    plt.legend()
    plt.savefig(os.path.join('figs', 'seq_length_vs_file_size.png'))


def generate_querysa_plots(df_seq, df_par):
    ''' Create the querysa experiment plots for part 2.
    '''

    fig, (ax1, ax2) = plt.subplots(1,2)
    ylim = df_seq['avg_query_time'].min(), df_seq['avg_query_time'].max()

    # Sequential query naive time plots 
    for prefix in df_seq['preftab'].unique():
        cur_df = df_seq[(df_seq['preftab'] == prefix) & (df_seq['mode'] == 'naive')]
        ax1.plot(cur_df['seq_length'], cur_df['avg_query_time'], marker='^', lw=2, label='k='+str(prefix))
    
    ax1.set_title('Naive')
    ax1.set_xlabel('# Base Pairs')
    ax1.set_ylabel('Avg. Time per Query (ms)')
    ax1.set_xscale('log', base=10)
    ax1.set_ylim(ylim)
    ax1.legend()

    # Sequential query simpleaccel time plots 
    for prefix in df_seq['preftab'].unique():
        cur_df = df_seq[(df_seq['preftab'] == prefix) & (df_seq['mode'] == 'simpleaccel')]
        ax2.plot(cur_df['seq_length'], cur_df['avg_query_time'], marker='^', lw=2, label='k='+str(prefix))
    
    ax2.set_title('Simple Accelerant')
    ax2.set_xlabel('# Base Pairs')
    ax2.set_xscale('log', base=10)
    ax2.get_yaxis().set_visible(False)
    ax2.set_ylim(ylim)

    fig.suptitle('Sequence Length vs Average Query Time')
    fig.savefig(os.path.join('figs', 'seq_length_vs_query_time.png'))



def average_experiments(df, key):
    ''' If there are multiple rows representing repetitions of an experiment,
        then this will aggregate them by `key` and average.
    '''
    return df.groupby(key).mean().reset_index()

def main(argc, argv):
    if argc != 5:
        print('usage: {} buildsa-sequential buildsa-parallel querysa-sequential querysa-parallel'.format(argv[0]), file=sys.stderr)
        exit(1)

    os.makedirs('figs', exist_ok=True)

    buildsa_seq_df = pd.read_csv(argv[1])
    buildsa_par_df = pd.read_csv(argv[2])
    querysa_seq_df = pd.read_csv(argv[3])
    querysa_par_df = pd.read_csv(argv[4])

    buildsa_seq_df = average_experiments(buildsa_seq_df, ['seq_length', 'preftab'])
    buildsa_seq_df['build_time'] = buildsa_seq_df['sa_build_time'] + buildsa_seq_df['preftab_build_time']
    buildsa_par_df = average_experiments(buildsa_par_df, ['seq_length', 'preftab'])
    buildsa_par_df['build_time'] = buildsa_par_df['sa_build_time'] + buildsa_par_df['preftab_build_time']

    querysa_seq_df = average_experiments(querysa_seq_df, ['seq_length', 'preftab', 'mode'])
    querysa_seq_df = average_experiments(querysa_seq_df, ['seq_length', 'preftab', 'mode'])

    generate_buildsa_plots(buildsa_seq_df, buildsa_par_df)
    generate_querysa_plots(querysa_seq_df, querysa_par_df)


if __name__ == '__main__':
    main(len(sys.argv), sys.argv)
