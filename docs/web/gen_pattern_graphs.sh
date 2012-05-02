#!/bin/bash

destdir=htdocs
idxname=pattern_index.html
imgtype=png

echo \<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN"\> > $destdir/$idxname
echo \<html\>\<head\>\</head\> >> $destdir/$idxname
echo \<body dir="ltr" bgcolor="#ffffff" lang="en-US"\> >> $destdir/$idxname

echo \<h1\>Inferno search and replace patterns\</h1\> >> $destdir/$idxname

i=0
while :
do
  imgname=pattern_$i.$imgtype
  ../../inferno.exe -gp$i -otemp.dot
  if [ $? -ne 0 ];
  then
    break
  fi
  dot -T$imgtype -o$destdir/$imgname temp.dot 
  echo \<p\> >> $destdir/$idxname
  echo \<h2\>Step $i\</h2\> >> $destdir/$idxname
  echo \</p\> >> $destdir/$idxname
  echo \<p\> >> $destdir/$idxname
  echo \<img src=\"$imgname\" alt=\"Pattern for step $i\"\> >> $destdir/$idxname
  echo \</p\> >> $destdir/$idxname
  i=$(( $i+1 ))
done

echo \</body\>\</html\> >> $destdir/$idxname

