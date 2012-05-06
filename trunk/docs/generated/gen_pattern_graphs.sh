#!/bin/bash

destdir=html
idxname=step_index.html
imgtype=png
testcase=test/examples/sctest13.cpp

echo \<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN"\> > $destdir/$idxname
echo \<html\>\<head\>\</head\> >> $destdir/$idxname
echo \<body dir="ltr" bgcolor="#ffffff" lang="en-US"\> >> $destdir/$idxname
echo \<h1\>Inferno search and replace patterns\</h1\> >> $destdir/$idxname

cd ../..
./inferno.exe -i$testcase -thSNnMFL > docs/generated/hits.txt 2> docs/generated/steps.txt
cd docs/generated

i=0
while :
do
  imgname=pattern_graph_$i.$imgtype
  stepname=step_$i.html
  
  # Generate graph of the pattern of the step
  ../../inferno.exe -gp$i -otemp.dot
  if [ $? -ne 0 ];
  then
    break
  fi
  dot -T$imgtype -o$destdir/$imgname temp.dot 
  mogrify -antialias -resize 35% $destdir/$imgname

  # Generate step-specific page
  grep "step $i " hits.txt > curhits.txt
  grep "Step $i:" steps.txt | sed 's/Step [0-9]*: Steps::\([a-zA-Z]*\)@0x[0-9a-f]*/\1/' > name.txt
  name=`cat name.txt`
  echo \<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN"\> > $destdir/$stepname
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
  done < curhits.txt
  echo \</body\>\</html\> >> $destdir/$stepname

  # Add link to link farm page
  echo \<p\>\<a href=\"$stepname\"\> >> $destdir/$idxname 
  echo "Step $i: $name" >> $destdir/$idxname
  echo \</a\>\</p\> >> $destdir/$idxname

  i=$(( $i+1 ))
done

echo -n "Generated " >> $destdir/$idxname
date >> $destdir/$idxname
echo \</body\>\</html\> >> $destdir/$idxname

