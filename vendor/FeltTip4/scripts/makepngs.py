#!/usr/bin/env python
# makepngs - Create arbitrarily sized pngs from the svg set.
#            Feel free to use this script in your own icon set!
# Platforms: Linux
# Depends on: Inkscape, Image Magick for border, pngcrush for compression
# Chromakode 2004-2005

import sys
import os, os.path
import commands
import shutil
import re

def no_nl_print(text):
    sys.stdout.write(text)
    sys.stdout.flush()
    
def run_quiet(cmd):
    """Run a command quietly, with no console output."""
    os.system(cmd + " > /dev/null")

def visit(args, root, files):
    size, target, skip = args
    for file in files:
        file, ext = os.path.splitext(file)
        if ext==".svg":
            pngname = os.path.join(target, file) + ".png"
            if os.path.exists(pngname):
                if skip:
                    no_nl_print(">")
                    continue
                else:
                    ovr=True
                    os.remove(pngname)
            else:  
                ovr=False
                
            # Render
            rendercmd="inkscape --without-gui --file=\"%s\" --export-png=\"%s\" --export-width=%s --export-height=%s" % (os.path.join(root, file)+ext,pngname, size, size)
            run_quiet(rendercmd)
            
            # Border
            if doborder:
                bordercmd="convert -border 1x1 -bordercolor transparent %s %s"%(pngname, pngname)
                run_quiet(bordercmd)
                
            # Crush
            if docrush:
                if dobrutecrush:
                    bruteswitch = "-brute"
                else:
                    bruteswitch = ""
                crushcmd="pngcrush %s %s %s2"%(bruteswitch, pngname, pngname)
                output = commands.getstatusoutput(crushcmd)[1]
                shutil.move(pngname+"2", pngname)
                # ewww...
                match = crushregex.search(output)
                # debug: print output, match is not None
                if match is not None:
                    # Compression worked
                    reduction = float(match.groups()[0])
                else:
                    reduction = 0
                reductionlist.append(reduction)
                
            if ovr:
                no_nl_print("+")
            else:
                no_nl_print("-")
                
size = raw_input("Sizes of icons? ")    
doborder = raw_input("Add space around edges? [y or N] ").upper()=="Y"
docrush = raw_input("Use pngcrush for additional compression? (lossless) [y or N] ").upper()=="Y"
if docrush:
    crushregex = re.compile(r"(\d.\d\d)% reduction")
    reductionlist = []
    print "Optionally you may use brute force pngcrush settings."
    print "Using this has the potential of making files even smaller, at the cost of"
    print "taking much longer per icon. This can be very slow on sizes over 64x64!"
    dobrutecrush = raw_input("Use brute force settings? [y or N] ").upper()=="Y"
target = raw_input("Target Directory? ")
skip = raw_input("Skip already rendered icons? [y or N] ").upper()=="Y"
print
print "---"
if not os.path.exists(target):
    print "Creating directory %s" % target
    os.mkdir(target)
else:
    print "Overwriting icons in old directory."
    
no_nl_print("Working: [")
os.path.walk('.',visit,(size,target,skip))
print("] done.")

if docrush and len(reductionlist)>0:
    print "Average pngcrush reduction: %s%%" % (round(sum(reductionlist)/len(reductionlist), 3))
