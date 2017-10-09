for f in testes/*.m;
do
   echo "testando $f"
   $1 $f
   clang out.ll
done
