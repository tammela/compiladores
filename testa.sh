#!/bin/bash

OUTPUT="output"
PROG="monga-cc"

for f in testes/*.monga; do
   echo "testando $f"
   tmp=$(mktemp tmp.XXX)
   $OUTPUT/$PROG $f > tmp
   if ! diff -q $tmp $f.gab; then
      echo "OK"
   else
      echo "NOT OK"
   fi
   rm $tmp
done

