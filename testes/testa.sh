for f in testes/*.m;
do
   echo "testando $f"
   tmp=$(mktemp testes/tmp.XXX)
   $1 $f > $tmp
   if diff -q $tmp $f.gab ; then
      echo "OK";
   else
      echo "NOT OK";
   fi
   rm $tmp
done
