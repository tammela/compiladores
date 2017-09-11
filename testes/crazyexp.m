foo() : int[][][][] {
   a : int[][][][];

   a = new int[10+10][10*10][10-10+1][10/10];
   return a;
}

bar() : char [] {
   return new char[10];
}

foobar() : void {
   while (1) {
      while (1) {
         @ 1 / 2 / 3 * 4 * (5 / 6);
      }
   }
}

barfoobar(a : int, b : float) : void
{
}

boo(a : int) : void {
}

barfoo() : int {
   return 10;
}

main() : void {
   bar()[0];
   if (!barfoo() - 5) {}
   barfoobar(1, 1.5);
   boo(1);
}
