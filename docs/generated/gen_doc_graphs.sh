#!/bin/bash

destdir=html
imgtype=svg
tempdir=temp
extraiopt=


mkdir -p $tempdir

# Run inferno to get the documentation graphs in dot form
../../inferno.exe -gd -o$tempdir/ $extraiopt

# Run dot on each dot file to get the image files
for fdot in $tempdir/*.dot
do
    fout=$destdir/`basename $fdot .dot`.$imgtype
	dot -T$imgtype $fdot > $fout
done

