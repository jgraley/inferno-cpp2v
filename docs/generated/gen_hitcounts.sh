#!/bin/bash -ex

destdir=html
idxname=step_index.html
imgtype=svg
testcase=test/examples/sctest13.cpp
extraiopt=

mkdir -p $destdir

echo \<!DOCTYPE html\> > $destdir/$idxname
echo \<html\>\<head\>\</head\> >> $destdir/$idxname
echo \<body dir="ltr" bgcolor="#ffffff" lang="en-US"\> >> $destdir/$idxname
echo \<h1\>Inferno search and replace patterns\</h1\> >> $destdir/$idxname

cd ../..
./inferno.exe $extraiopt -i$testcase -thSMFL > docs/generated/hits.txt 2> docs/generated/steps.txt
cd docs/generated

i=0
while :
do
  progress=T$i
  imgname=pattern_graph_$progress.$imgtype
  stepname=$progress.html
  
  # Generate graph of the pattern of the step
  set +e
  ../../inferno.exe -gp$i -otemp.dot
  if [ $? -ne 0 ];
  then
    break
  fi
  set -e
  
  dot -T$imgtype -o$destdir/$imgname temp.dot 
  # transform image size - not needed if vector eg svg
  # mogrify -antialias -resize 40% $destdir/$imgname

  # Generate step-specific page
  grep "At $progress " hits.txt > cur_hits.txt
  grep "at $progress:" steps.txt | sed 's/Step [0-9]*: Steps::\([a-zA-Z]*\)@0x[0-9a-f]*/\1/' > name.txt
  name=`cat name.txt`
  echo \<!DOCTYPE html\> > $destdir/$stepname
  echo \<html\>\<head\>\</head\> >> $destdir/$stepname
  echo \<body dir="ltr" bgcolor="#ffffff" lang="en-US"\> >> $destdir/$stepname
  echo \<p\> >> $destdir/$stepname
  echo \<h2\>Step $i\</h2\> >> $destdir/$stepname
  echo \<a href=\"classSteps_1_1$name.html\"\>$name\</a\> >> $destdir/$stepname
  echo \</p\> >> $destdir/$stepname
  echo \<p\> >> $destdir/$stepname
  echo \<img src=\"$imgname\" alt=\"Pattern for step $i\"\> >> $destdir/$stepname
  echo \</p\> >> $destdir/$stepname  
  echo Hit counts breakdown based on $testcase\<br\> >> $destdir/$stepname   
  while read p; do
    echo $p\<br\> >> $destdir/$stepname
  done < cur_hits.txt
  echo \</p\> >> $destdir/$stepname  

  echo \</body\>\</html\> >> $destdir/$stepname

  # Add link to link farm page
  echo \<p\>\<a href=\"$stepname\"\> >> $destdir/$idxname 
  echo "At $i: $name" >> $destdir/$idxname
  echo \</a\>\</p\> >> $destdir/$idxname

  i=$(( $i+1 ))
done

echo -n "Generated " >> $destdir/$idxname
date >> $destdir/$idxname
echo \</body\>\</html\> >> $destdir/$idxname

