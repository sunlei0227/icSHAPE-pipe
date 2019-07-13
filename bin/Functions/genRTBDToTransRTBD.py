#!/usr/bin/env python
#-*- coding:utf-8 -*-


import sys
import numpy
import getopt
import os
import version

dirname = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, dirname+'/../../GAP')

import GAP

Usage = """
genRTBDToTransRTBD - Convert genome tab-separated RTBD (.gTab) to transcript-based RTBD
=======================================================================================
\x1b[1mUSAGE:\x1b[0m 
  %s [-g genomeCoor.bed | -s genomeSize] -i input.tab -o out_file

\x1b[1mHELP:\x1b[0m
  -i                    <String>
                                Input a genome-based SHAPE file (produced by calc_sliding_shape)
  -g                    <String>
                                Genome mode
                                A genome-coor based annotation file produced by GAP: hg38.genomeCoor.bed (generated by parseGTF)
                                -s and -g cannot be specified at the same time
  -s                    <String>
                                Transcript mode
                                Specify the transcript size file
                                -s and -g cannot be specified at the same time
  -p                    <Int>
                                ≥1, number of processes, each process consumes the same amount of memory (default: 5)
  -c or --col           <Int Array>
                                Columns to output. Examples: -c 4,5,6... or -c 4-7,9-10...
  -o                    <String>
                                Output file

\x1b[1mVERSION:\x1b[0m
    %s

\x1b[1mAUTHOR:\x1b[0m
    Li Pan

""" % (sys.argv[0], version.Version)

def parser_columns(raw_string):
    columns = []
    for item in raw_string.split(','):
        if '-' in item:
            start = int(item.split('-')[0])
            end = int(item.split('-')[1])
            for i in range(start, end+1):
                columns.append(i)
        else:
            columns.append(int(item))
    return columns

def init():
    params = { 'threads': 5 }
    opts, args = getopt.getopt(sys.argv[1:], 'hi:g:o:s:p:c:', ['col='])
    for op, value in opts:
        if op == '-h':
            print(Usage)
            sys.exit(-1)
        
        elif op == '-i':
            params['input'] = os.path.abspath(value)
        elif op == '-g':
            params['annotation'] = os.path.abspath(value)
        elif op == '-o':
            params['output'] = value
        elif op == '-p':
            params['threads'] = int(value)
        elif op == '-s':
            params['sizeFile'] = value
        elif op == '--col' or op == '-c':
            params['columns'] = parser_columns(value)
        else:
            sys.stderr.writelines("Error: unrecognized parameter: "+op+"\n")
            print(Usage)
            sys.exit(-1)
    # check
    if ('input' not in params) or ('output' not in params) or ('columns' not in params):
        sys.stderr.writelines("Error: Please specify -i -o --col"+"\n")
        print(Usage)
        sys.exit(-1)
    if ('annotation' not in params) and ('sizeFile' not in params):
        sys.stderr.writelines("Error: Please specify -g or -s"+"\n")
        print(Usage)
        sys.exit(-1)
    if ('annotation' in params) and ('sizeFile' in params):
        sys.stderr.writelines("Error: -s and -g cannot be specified simultaneously"+"\n")
        print(Usage)
        sys.exit(-1)
    return params

def readGenomeSHAPE(inFile, columns):
    count = 0
    genomeSHAPE = {}
    for line in open(inFile):
        if line[0] == '@': continue
        count += 1
        if count % 100000 == 0:
            print("\tlines"+str(count))
        data = line.strip().split()
        chr_id = data[0]
        strand = data[1]
        pos = int(data[2])
        RTBD = ",".join( [ data[i-1] for i in columns ] )
        try:
            genomeSHAPE[chr_id].append( (chr_id, strand, pos, RTBD) )
        except KeyError:
            genomeSHAPE[chr_id] = []
            genomeSHAPE[chr_id].append( (chr_id, strand, pos, RTBD) )
    
    return genomeSHAPE

def genomeSHAPETotransSHAPE(chrSHAPEList, Parser, outFile):
    transSHAPE = {}
    for chr_id, strand, pos, RTBD in chrSHAPEList:
        try:
            trans_location_list = Parser.genomeCoor2transCoor(chr_id, pos, pos, strand)
        except:
            continue
        for trans_location in trans_location_list:
            tid = trans_location[3]
            trans_pos = trans_location[4]
            if tid not in transSHAPE:
                length = Parser.getTransFeature(tid)['trans_len']
                if length < 40: continue
                transSHAPE[tid] = ['NULL'] * length
            transSHAPE[tid][trans_pos-1] = RTBD
    
    OUT = open(outFile, 'w')
    for tid in transSHAPE:
        valid_base = len([it for it in transSHAPE[tid] if it != 'NULL'])
        if valid_base < 10: continue
        OUT.writelines( "%s\t%s\t" % (tid, len(transSHAPE[tid])) )
        OUT.writelines( "\t".join(transSHAPE[tid]) )
        OUT.writelines("\n")
    OUT.close()

def genomeSHAPEToGenomeSHAPE(genomeSHAPEList, ChrSize, outFile):
    OUT = open(outFile, 'w')
    for chr_id in genomeSHAPEList:
        cur_chr = ['NULL'] * ChrSize[chr_id]
        OUT.writelines("%s\t%s\t" % (chr_id, ChrSize[chr_id]))
        for chr_id,strand,pos,RTBD in genomeSHAPEList[chr_id]:
            if strand == '+':
                cur_chr[pos-1] = RTBD
        OUT.writelines( "\t".join(cur_chr) )
        OUT.writelines( "\n" )
    
    OUT.close()

def parallel_genomeSHAPETotransSHAPE(genomeSHAPE, Parser, final_outFile, threads=5):
    from multiprocessing import Process
    import random
    
    randID = random.randint(1, 10000)
    
    chr_left = list(genomeSHAPE.keys())
    chr_left.sort(key=lambda x: len(genomeSHAPE[x]))
        
    OUT = open(final_outFile, 'w')
    while len(chr_left) > 0:
        sys.stdout.write("%s\t" % (len(chr_left), ))
        threads_left = threads
        process_list = []
        while len(chr_left) > 0 and threads_left > 0:
            cur_chr = chr_left[0]
            sys.stdout.write('\t'+cur_chr)
            outFile = "/tmp/%s_%s.shape" % (cur_chr, randID)
            
            p = Process(target=genomeSHAPETotransSHAPE, args=(genomeSHAPE[cur_chr], Parser, outFile))
            process_list.append( (p,outFile,cur_chr) )
            
            del chr_left[0]
            threads_left -= 1
        
        sys.stdout.write('\n'); sys.stdout.flush()
        for p,outFile,cur_chr in process_list:
            p.start()
        for p,outFile,cur_chr in process_list:
            p.join()
            append_file(open(outFile), OUT)
            os.remove(outFile)
        for p,outFile,cur_chr in process_list:
            del genomeSHAPE[cur_chr]
    
    OUT.close()

def load_chr_size(inFile):
    chrSize = {}
    for line in open(inFile):
        arr = line.strip().split()
        chrSize[arr[0]] = int(arr[1])
    return chrSize

def append_file(IN, OUT):
    for line in IN:
        OUT.writelines(line)

def main():
    parameters = init()
    
    print("Start to load gTab SHAPE...")
    genomeSHAPE = readGenomeSHAPE(parameters['input'], parameters['columns'])
    
    if 'sizeFile' not in parameters:
        
        print("Start to read annotations...")
        Parser = GAP.init(parameters['annotation'])
        
        print("Start to transform coordination system...")
        parallel_genomeSHAPETotransSHAPE(genomeSHAPE, Parser, parameters['output'], threads=parameters['threads'])
    
    else:
        
        print("Start to load chr size...")
        chrSize = load_chr_size(parameters['sizeFile'])
        
        print("Start to build Genome SHAPE...")
        genomeSHAPEToGenomeSHAPE(genomeSHAPE, chrSize, parameters['output'])
    
    print("Success!")

main();





