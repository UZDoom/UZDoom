#!/usr/bin/env python3

import sys

execpath, inpath, outpath, *dict_list = sys.argv

dictionary = {}
while dict_list:
    key, value, *rest = dict_list
    dictionary[key] = value
    dict_list = rest

infile = open(inpath, 'r')
outfile = open(outpath, 'w')

buf = infile.read()
infile.close()

for key, value in dictionary.items():
    buf = buf.replace('@{}@'.format(key), value)

outfile.write(buf)
outfile.close()
