# for lines with a "start-of-message" header
# (We will be fooled by a message containing such a header, such as
# "forwarded" messages, but that's a defect of Licq history files and
# can't be helped.)
/.+ from .* (received|sent) .+:/ {	# regexp could be improved...
    if (hitp) {
	# print previous message if it was a hit
	printbuf(buf, bufc)
	print ""
    }
    # clear buffer line counter and hit flag
    bufc = 0
    hitp = 0
}

# for all lines, until matching line encountered
!hitp {
    if (NR in rows) {
	# current line is a matching line, so set flag
	hitp = 1
    }
}

# for all lines
{
    # add current line to buffer
    buf[bufc++] = $0
}

END {
    if (hitp) {
	# print the last message in the file, if it was a hit
	printbuf(buf, bufc)
    }
}

