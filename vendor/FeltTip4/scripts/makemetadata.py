#!/usr/bin/env python
# makemetadata - Copy the metadata from one SVG to a directory of SVGs
# Platforms: Linux, others?
# Depends on: Python-XML
# Chromakode 2004-2005

import os
from xml.dom import minidom

def visit(metadata, root, files):
    for file in files:
        file, ext = os.path.splitext(file)
        if ext==".svg":
            filexml = minidom.parse(os.path.join(root, file)+ext) 
            found = filexml.getElementsByTagName("metadata")
            if len(found)==1:
                found[0].parentNode.replaceChild(metadata, found[0])
            else:
                filexml.getElementsByTagName("svg")[0].appendChild(metadata)
            f = open(os.path.join(root, file)+ext, "w")
            f.write(filexml.toxml())
            f.close()

goodtemplate = False
while not goodtemplate:           
    template = raw_input("Base SVG? ")
    if os.path.exists(template):
        templatexml = minidom.parse(template)
        found = templatexml.getElementsByTagName("metadata")
        if len(found)==1:
            metadata = found[0]
            goodtemplate = True
        else:
            print "Unable to locate metadata in %s." % template
    else:
        print "Unable to access %s." % template
        
print "Working..."
os.path.walk('.',visit,metadata)
print "done."
