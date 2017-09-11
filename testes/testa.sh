for f in testes/*.m;
do
   echo "testando $f"
   tmp=$(mktemp testes/tmp.XXX)
   $1 $f > $tmp 2>/dev/null
   if diff -Z -q $tmp $f.gab ; then
      echo "OK";
   else
      echo "NOT OK";
   fi
#   rm $tmp
done
