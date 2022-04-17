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
    
    # Build time plots
    plt.plot(df_seq['preftab'], df_seq['build_time'], lw=2, color='red', label='Serial')
    plt.plot(df_par['preftab'], df_par['build_time'], lw=2, color='blue', label='OpenMP')

    plt.title('Size of Prefix in Prefix Table vs Build Time')
    plt.xlabel('Prefix Length')
    plt.ylabel('Time (ms)')
    plt.xscale('log', base=2)
    plt.legend()

    plt.savefig(os.path.join('figs', 'preftab_vs_build_time.png'))

    # File size plots
    plt.clf()
    plt.plot(df_seq['preftab'], df_seq['file_size'] / 1E6, lw=2, color='blue')

    plt.title('Size of Prefix in Prefix Table vs File Size')
    plt.xlabel('Prefix Length')
    plt.ylabel('File Size (MB)')
    plt.xscale('log', base=2)

    plt.savefig(os.path.join('figs', 'preftab_vs_file_size.png'))



def average_experiments(df, key):
    ''' If there are multiple rows representing repetitions of an experiment,
        then this will aggregate them by `key` and average.
    '''
    return df.groupby(key).mean().reset_index()

def main(argc, argv):
    if argc != 4:
        print('usage: {} buildsa-sequential buildsa-parallel querysa'.format(argv[0]), file=sys.stderr)
        exit(1)

    os.makedirs('figs', exist_ok=True)

    buildsa_seq_df = pd.read_csv(argv[1])
    buildsa_par_df = pd.read_csv(argv[2])

    buildsa_seq_df = average_experiments(buildsa_seq_df, ['seq_length', 'preftab'])
    buildsa_seq_df['build_time'] = buildsa_seq_df['sa_build_time'] + buildsa_seq_df['preftab_build_time']
    buildsa_par_df = average_experiments(buildsa_par_df, ['seq_length', 'preftab'])
    buildsa_par_df['build_time'] = buildsa_par_df['sa_build_time'] + buildsa_par_df['preftab_build_time']

    generate_buildsa_plots(buildsa_seq_df, buildsa_par_df)


if __name__ == '__main__':
    main(len(sys.argv), sys.argv)
