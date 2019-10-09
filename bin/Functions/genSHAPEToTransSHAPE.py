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
genSHAPEToTransSHAPE - Convert genome tab-separated SHAPE (.gTab) to transcript-based SHAPE
===========================================================================================
\x1b[1mUSAGE:\x1b[0m 
  %s [-g genomeCoor.bed | -s genomeSize] -i input.gTab -o out_file

\x1b[1mHELP:\x1b[0m
  -i                    <String>
                                Input a gTab file (produced by calc_sliding_shape)
  -g                    <String>
                                Genome mode
                                A genome-coor based annotation file produced by GAP: hg38.genomeCoor.bed (generated by parseGTF)
                                -s and -g cannot be specified at the same time
  -s                    <String>
                                Transcript mode
                                specify the transcript size file
                                -s and -g cannot be specified at the same time
  -p                    <Int>
                                ≥1, number of processes, each process consumes the same amount of memory (default: 5)

  Output options:
  -c                    <Int>
                                Minimun coverage for valid shape score (default: 200)
  -T                    <Float>
                                Minimun averaged RT for valid transcript (default: 2)
  -M                    <Float>
                                Minimun FPKM for valid transcript (default: 5)
  -n                    <Int>
                                Minimun number of covered base for valid transcript (default: 10)
  -m                    <Float>
                                0-1, minimum ratio of covered base for valid transcript (default: 0.1)
  -r                    <String,String,...>
                                Isoform FRPKM file generated by cufflinks, such as isoforms.fpkm_tracking, multiple files can be specified
  -o                    <String>
                                Output a SHAPE.out file
   
   More options:
   --app                <none>
                                Append to file

\x1b[1mVERSION:\x1b[0m
    %s

\x1b[1mAUTHOR:\x1b[0m
    Li Pan

""" % (sys.argv[0], version.Version)


def init():
    params = { 'minBD': 200, 'minAveRT': 2.0, 'minCovNum': 10, 'minCovRatio': 0.1, 'minFPKM':5.0, 'threads': 5, 'append': False }
    opts, args = getopt.getopt(sys.argv[1:], 'hi:g:s:p:c:T:M:n:m:r:o:', ['app'])
    for op, value in opts:
        if op == '-h':
            print(Usage)
            sys.exit(-1)
        # Basic Parameters
        elif op == '-i':
            params['input'] = os.path.abspath(value)
        elif op == '-g':
            params['annotation'] = os.path.abspath(value)
        elif op == '-s':
            params['sizeFile'] = value
        elif op == '-p':
            params['threads'] = int(value)
        
        elif op == '-c':
            params['minBD'] = int(value)
        elif op == '-T':
            params['minAveRT'] = float(value)
        elif op == '-M':
            params['minFPKM'] = float(value)
        elif op == '-n':
            params['minCovNum'] = float(value)
        elif op == '-m':
            params['minCovRatio'] = float(value)
        elif op == '-r':
            params['fpkmFile'] = value
        elif op == '-o':
            params['output'] = value
        
        elif op == '--app':
            params['append'] = True
        
        else:
            sys.stderr.writelines("Error: unrecognized parameter: "+op+"\n")
            print(Usage)
            sys.exit(-1)
    # check
    if ('input' not in params) or ('output' not in params):
        sys.stderr.writelines("Error: Please specify -i -o"+"\n")
        print(Usage)
        sys.exit(-1)
    if ('annotation' not in params) and ('sizeFile' not in params):
        sys.stderr.writelines("Error: Please specify -g or -s"+"\n")
        print(Usage)
        sys.exit(-1)
    if ('annotation' in params) and ('sizeFile' in params):
        sys.stderr.writelines("Error:  -s and -g cannot be specified at the same time"+"\n")
        exit(-1)
    
    return params



def read_gTab_head(IN):
    last_pos = IN.tell()
    
    gTab_head = {}
    
    line = IN.readline()
    while line:
        if not line.startswith('@'):
            ### Check column nums
            data = line.strip().split()
            if len(data) != gTab_head['ColNum']:
                sys.stderr.writelines("Error: actual column number (%s) != labeled number (%s)" % (len(data), gTab_head['ColNum'])+"\n")
                exit(-1)
            IN.seek(last_pos)
            break
        tag, num = line.strip()[1:].split()
        if tag == 'ColNum':
            gTab_head['ColNum'] = int(num)
        elif tag == 'ChrID':
            gTab_head['ChrID'] = int(num)
        elif tag == 'Strand':
            gTab_head['Strand'] = int(num)
        elif tag == 'ChrPos':
            gTab_head['ChrPos'] = int(num)
        elif tag == 'N_RT':
            gTab_head['N_RT'] = int(num)
        elif tag == 'N_BD':
            gTab_head['N_BD'] = int(num)
        elif tag == 'D_RT':
            gTab_head['D_RT'] = int(num)
        elif tag == 'D_BD':
            gTab_head['D_BD'] = int(num)
        elif tag == 'Shape':
            gTab_head['Shape'] = int(num)
        elif tag == 'ShapeNum':
            gTab_head['ShapeNum'] = int(num)
        elif tag == 'WindowShape':
            gTab_head['WindowShape'] = int(num)
        elif tag == 'Base':
            gTab_head['Base'] = int(num)
        else:
            sys.stderr.writelines("Warning: Unknown head tag: "+line.strip()+"\n")
        
        last_pos = IN.tell()
        line = IN.readline()
    
    return gTab_head


def read_A_ChrStrand(IN, gTab_head, minBD):
    chrID_col = gTab_head['ChrID']
    strand_col = gTab_head['Strand']
    chrPos_col = gTab_head['ChrPos']
    nRT_col = gTab_head['N_RT']
    nBD_col = gTab_head['N_BD']
    shape_col = gTab_head['Shape']
    
    if 'D_BD' in gTab_head:
        BD_col = gTab_head['D_BD']
    else:
        BD_col = nBD_col
    
    last_pos = IN.tell()
    
    cur_chr_id = ""
    cur_chr_strand = ""
    genomeSHAPEList = []
    
    line = IN.readline()
    while line:
        data = line.strip().split()
        chr_id = data[ chrID_col-1 ]
        chr_strand = data[ strand_col-1 ]
        chr_pos = int(data[ chrPos_col-1 ])
        BD = int(data[ BD_col-1 ])
        RT = int(data[ nRT_col-1 ])
        shape = data[ shape_col-1 ]
        
        if not cur_chr_id:
            cur_chr_id = chr_id
            cur_chr_strand = chr_strand
        else:
            if chr_id != cur_chr_id or chr_strand != cur_chr_strand:
                IN.seek(last_pos)
                break
        
        if BD>=minBD and shape!='-1':
            shape = "%.3f" % (float(shape), )
        else:
            shape = "NULL"
        
        if shape == '-1':
            shape = 'NULL'
        
        if RT>=1 or shape != 'NULL':
            genomeSHAPEList.append( (chr_pos, shape, RT) )
        
        last_pos = IN.tell()
        line = IN.readline()
    
    if not line and len(genomeSHAPEList)==0:
        return False, False, False
    else:
        return cur_chr_id, cur_chr_strand, genomeSHAPEList

def genomeSHAPE2TransSHAPE(chr_id, strand, genomeSHAPEList, Parser, outFile):
    transSHAPE = {}
    
    # count and record
    intergenic = 0
    intron = 0
    exon = 0
    
    for pos,shape,RT in genomeSHAPEList:
        
        try:
            trans_location_list = Parser.genomeCoor2transCoor(chr_id, pos, pos, strand)
        except:
            continue
        
        if len(trans_location_list) == 0:
            try:
                gene_location_list = Parser.genomeCoor2geneCoor(chr_id, pos, pos, strand)
            except:
                continue
            if len(gene_location_list) == 0:
                intergenic += 1
            else:
                intron += 1
        else:
            exon += 1
        
        for trans_location in trans_location_list:
            tid = trans_location[3]
            trans_pos = trans_location[4]
            
            if tid not in transSHAPE:
                length = Parser.getTransFeature(tid)['trans_len']
                if length < 40: continue
                transSHAPE[tid] = [('NULL', 'NULL')] * length
            
            transSHAPE[tid][trans_pos-1] = (shape, RT)
    
    #### output
    OUT = open(outFile, 'w')
    
    OUT.writelines("#%s\t%s\t%s\t%s" % (chr_id+strand, exon, intergenic, intron)+"\n")
    for tid in transSHAPE:
        valid_shape_num = len([ it for it in transSHAPE[tid] if it[0]!='NULL' ])
        
        if valid_shape_num > 2:
            
            shape_str = tid
            for shape_RT in transSHAPE[tid]:
                shape_str += "\t%s,%s" % (shape_RT[0], shape_RT[1])
            
            OUT.writelines(shape_str+'\n')
    
    OUT.close()

def readTmpShape(tmp_shape):
    icSHAPE = {}
    
    for line in open(tmp_shape):
        if line.startswith('#'):
            continue
        
        arr = line.strip().split()
        trans_id = arr[0]
        shape_RT_List = arr[1:]
        icSHAPE[ trans_id ] = [ it.split(',') for it in shape_RT_List ]
    
    return icSHAPE

def saveTransSHAPE(transSHAPE, outFile, append, FPKMDict=None):
    if append:
        OUT = open(outFile, 'a')
    else:
        OUT = open(outFile, 'w')
    
    for tid in transSHAPE:
        length = len(transSHAPE[tid])
        
        fpkm = '*'
        if FPKMDict != None:
            fpkm = "%.3f" % (FPKMDict[tid], )
        
        OUT.writelines("%s\t%s\t%s\t%s\n" % (tid, length, fpkm, "\t".join(transSHAPE[tid])))
    
    OUT.close()

def filterSHAPE(inputSHAPE, minCovRatio=0.1, minCovNum=10, minAveRT=2, minFPKM=5, FPKMDict=None):
    filtSHAPE = {}
    for tid in inputSHAPE:
        length = len(inputSHAPE[tid])
        shape_list = [ str(it[0]) for it in inputSHAPE[tid] ]
        RT_list = [ int(it[1]) for it in inputSHAPE[tid][5:-30] if it[1] != 'NULL' ]
        
        valid_shape_len = len(shape_list[5:-30]) - shape_list[5:-30].count('NULL')
        if 1.0*valid_shape_len/length < minCovRatio:
            continue
        if 1.0*sum([int(it) for it in RT_list])/(length-35) < minAveRT:
            continue
        if valid_shape_len < minCovNum:
            continue
        if FPKMDict != None:
            if tid not in FPKMDict:
                continue
            if FPKMDict[tid] < minFPKM:
                continue
        
        for i in range(5):
            shape_list[i] = 'NULL'
        for i in range(length-30, length):
            shape_list[i] = 'NULL'
        
        filtSHAPE[tid] = shape_list
    
    return filtSHAPE


def loadChrStatistics(inFile):
    statistics = []
    first_line = open(inFile).readline()
    
    if not first_line.startswith('#'):
        sys.stderr.writelines("Warning: "+inFile+" doesn't statistics"+"\n")
        return ["Unknown", 0, 0, 0]
    
    data = first_line[1:].strip().split()
    return [ data[0], int(data[1]), int(data[2]), int(data[3]) ]


def parallel_genomeSHAPE2TransSHAPE(inFile, Parser, minBD=100, threads=5, minCovRatio=0.1, minCovNum=10, minAveRT=2, minFPKM=5, FPKMDict=None):
    from multiprocessing import Process
    import random
    
    randID = random.randint(1, 10000)
    root_path = "/tmp/shape-pipe_%s/" % (randID, )
    if not os.path.exists(root_path):
        os.mkdir(root_path)
    
    if inFile.endswith(".gz"):
        import gzip
        IN = gzip.open(inFile, 'r')
    else:
        IN = open(inFile, 'r')
    
    gTab_head = read_gTab_head(IN)
    
    transSHAPE = {}
    countList = []
    
    ############### Init ###############
    process_list = []
    while len(process_list) < threads:
        chr_id, chr_strand, genomeSHAPEList = read_A_ChrStrand(IN, gTab_head, minBD=minBD)
        if not chr_id: break
        
        print("process chr "+chr_id)
        tmpFile = root_path+"%s_%s_%s.shape" % (chr_id, chr_strand, randID)
        p = Process(target=genomeSHAPE2TransSHAPE, args=(chr_id, chr_strand, genomeSHAPEList, Parser, tmpFile))
        p.start()
        process_list.append( (p, tmpFile) )
    
    ############### Append ###############
    while len(process_list) != 0:
        p1, tmpFile = process_list[0]
        p1.join()
        
        rawSHAPE = readTmpShape(tmpFile)
        filtSHAPE = filterSHAPE(rawSHAPE, minCovRatio=minCovRatio, minCovNum=minCovNum, minAveRT=minAveRT, minFPKM=minFPKM, FPKMDict=FPKMDict)
        transSHAPE.update( filtSHAPE )
        countList.append( loadChrStatistics(tmpFile) )
        
        del process_list[0]
        chr_id, chr_strand, genomeSHAPEList = read_A_ChrStrand(IN, gTab_head, minBD=minBD)
        if chr_id:
            print("process chr "+chr_id)
            tmpFile = root_path+"%s_%s.shape" % (chr_id, chr_strand)
            p = Process(target=genomeSHAPE2TransSHAPE, args=(chr_id, chr_strand, genomeSHAPEList, Parser, tmpFile))
            p.start()
            process_list.append( (p, tmpFile) )
    
    # output map result
    countList.sort(key=lambda x: x[0])
    print("chr_id\texon\tintergenic\tintron\texon_ratio")
    for chr_id,exon,intergenic,intron in countList:
        print("%s\t%s\t%s\t%s\t%.3f%%" % (chr_id, exon, intergenic, intron, 100.0*exon/(exon+intergenic+intron+1)))
    t_exon = sum([it[1] for it in countList])
    t_intergentic = sum([it[2] for it in countList])
    t_intron = sum([it[3] for it in countList])
    print("\n###Total\t%s\t%s\t%s\t%.3f%%" % (t_exon, t_intergentic, t_intron, 100.0*t_exon/(t_exon+t_intergentic+t_intron)))
    
    os.system("rm -r "+root_path)
    
    return transSHAPE

def loadFPKM(inFile):
    FPKM = {}
    IN = open(inFile)
    IN.readline()
    line = IN.readline()
    while line:
        data = line.strip().split()
        tid = data[0]
        fpkm = float(data[9])
        status = data[12]
        if status == 'OK':
            FPKM[tid] = fpkm
        line = IN.readline()
    return FPKM

def batch_loadFPKM(fileList):
    import numpy
    
    FPKM = {}
    for file in fileList:
        cur_fpkm = loadFPKM(file)
        for tid in cur_fpkm:
            FPKM[tid] = FPKM.get(tid, []) + [cur_fpkm[tid]]
    for tid in FPKM:
        FPKM[tid] = numpy.mean(FPKM[tid])
    return FPKM


def load_chr_size(inFile):
    chrSize = {}
    for line in open(inFile):
        arr = line.strip().split()
        chrSize[arr[0]] = int(arr[1])
    return chrSize

def genomeSHAPEToGenomeSHAPE(inFile, ChrSize, minBD):
    genomeSHAPE = {}

    IN = open(inFile)
    gTab_head = read_gTab_head(IN)

    cur_chr_id, cur_chr_strand, genomeSHAPEList = read_A_ChrStrand(IN, gTab_head, minBD=minBD)
    while cur_chr_id:
        if cur_chr_strand == '+':
            # (chr_pos, shape, RT)
            if cur_chr_id not in genomeSHAPE:
                genomeSHAPE[ cur_chr_id ] = [ ('NULL', 'NULL') ] * ChrSize[cur_chr_id]
            for (chr_pos, shape, RT) in genomeSHAPEList:
                genomeSHAPE[ cur_chr_id ][ chr_pos-1 ] = (shape, RT)
        cur_chr_id, cur_chr_strand, genomeSHAPEList = read_A_ChrStrand(IN, gTab_head, minBD=minBD)

    return genomeSHAPE



def main():
    params = init()
    
    fpkm = None
    if 'fpkmFile' in params:
        print("Start to load fpkm file...")
        fpkm = batch_loadFPKM(params['fpkmFile'].split(','))
    
    if 'annotation' in params:
        print("Start to load annotation file...")
        GAPer = GAP.init(params['annotation'], showAttr=False)
        transSHAPE = parallel_genomeSHAPE2TransSHAPE(params['input'], GAPer,    minBD=params['minBD'], threads=params['threads'], \
                                                                                minCovRatio=params['minCovRatio'], minCovNum=params['minCovNum'], \
                                                                                minAveRT=params['minAveRT'], minFPKM=params['minFPKM'], FPKMDict=fpkm)
        saveTransSHAPE(transSHAPE, params['output'], params['append'], FPKMDict=fpkm)
    else:
        ChrSize = load_chr_size(params['sizeFile'])
        genomeSHAPE = genomeSHAPEToGenomeSHAPE(params['input'], ChrSize, minBD=params['minBD'])
        filt_genomeSHAPE = filterSHAPE(genomeSHAPE, minCovRatio=params['minCovRatio'], minCovNum=params['minCovNum'], minAveRT=params['minAveRT'], minFPKM=params['minFPKM'], FPKMDict=fpkm)
        saveTransSHAPE(filt_genomeSHAPE, params['output'], params['append'], FPKMDict=fpkm)


if __name__ == "__main__":
    main()

"""
hg38_parser = GAP.init("/150T/zhangqf/GenomeAnnotation/Gencode/hg38.genomeCoor.bed")
testFile = "/Share/home/zhangqf8/lipan/icSHAPE-pipe/new_pipeline/4.shape_score/test.txt"
FPKM = batch_loadFPKM(['/Share/home/zhangqf8/lipan/icSHAPE-pipe/new_pipeline/3.rpkm/D1/isoforms.fpkm_tracking', '/Share/home/zhangqf8/lipan/icSHAPE-pipe/new_pipeline/3.rpkm/D2/isoforms.fpkm_tracking'])
transSHAPE = parallel_genomeSHAPE2TransSHAPE(testFile, hg38_parser, minBD=200, threads=5, minCovRatio=0.1, minCovNum=10, minAveRT=2, minFPKM=5, FPKMDict=FPKM)
saveTransSHAPE(transSHAPE, "/tmp/test", FPKMDict=FPKM)
"""


