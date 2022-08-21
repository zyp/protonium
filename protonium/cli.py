import argparse

from . import include

def cli():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cflags', action = 'store_true', help = 'Output CFLAGS with protonium include path.')
    args = parser.parse_args()

    if args.cflags:
        print(f'-I{ include.__dir__ }')
