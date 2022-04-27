import random

input = 'inputs/sars-cov-2.fa'
output = 'inputs/sars-cov-2-queries.fa'
#input = 'inputs/GRCh38_latest_genomic.fa'
#output = 'inputs/GRCh38_latest_genomic-queries.fa'
#input = 'inputs/fruitfly.fa'
#output = 'inputs/fruitfly-queries.fa'
#input = 'inputs/fungus.fa'
#output = 'inputs/fungus-queries.fa'
NUM_QUERIES = 10000
QUERY_LEN = (20, 40)

def get_random_str(main_str, substr_len):
    idx = random.randrange(0, len(main_str) - substr_len + 1)    # Randomly select an "idx" such that "idx + substr_len <= len(main_str)".
    return main_str[idx : (idx+substr_len)]

def main():
    sequence = ''
    with open(input, 'r') as fp:
        sequence = map(lambda s: s.strip(), fp.readlines()[1:])
        sequence = ''.join(sequence)

    with open(output, 'w') as fp:
        for q in range(NUM_QUERIES):
            q_len = random.randrange(*QUERY_LEN)
            query = get_random_str(sequence, q_len).upper()

            while 'N' in query:
                query = get_random_str(sequence, q_len).upper()

            fp.write('>{}:{}\n'.format(q, q_len))
            fp.write('{}\n'.format(query))


if __name__ == '__main__':
    main()
