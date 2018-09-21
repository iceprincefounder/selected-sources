#!/usr/bin/python


import os
from argparse import ArgumentParser

def argsChaser():
    parser = ArgumentParser(usage='\t Batch convert EXR file to new and autocrop data window.')
    parser.add_argument("-f", "--filePath", metavar=' *./ ', action='store', default="")
    parser.add_argument("-a", "--all", metavar='', action='store', default="", help="convert all!")
    args=None
    temp_args,unknow = parser.parse_known_args()
    args=vars(temp_args)
    return args

args = argsChaser()
root = os.path.dirname(os.path.realpath(__file__))

def main():
    file_path = "/home/xukai/Documents/TestDataFactory/EXR_Crop_Test/source/chr_a_crop"
    all_files = os.listdir(file_path)
    i = 0
    for file in all_files:
        print "------------------"
        i += 1
        print "%s/%s"%(i,len(all_files))
        print "File: %s"%file
        if not file.endswith(".exr"):
            continue
        excute=os.path.join(root, "application","bin", "EXRAutoCropper")
        infile = os.path.join(file_path,file)
        # outfile = os.path.join(file_path,"crop",file)        
        outfile = infile
        command = "%s %s %s"%(excute, infile, outfile)
        os.system(command)
if __name__ == "__main__":
    main()