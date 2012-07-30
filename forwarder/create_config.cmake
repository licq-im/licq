# Read INPUT, escape quotes and generate a cpp file called OUTPUT
file(READ ${INPUT} raw)
string(REPLACE "\"" "\\\"" escaped ${raw})
string(REGEX REPLACE "\n" "\\\\n\"\n\"" quoted ${escaped})
file(WRITE ${OUTPUT}
    "extern const char* const FORWARDER_CONF =\n\"${quoted}\";\n")
