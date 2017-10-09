# it should be OK

foo () : void {}

main() : void {
   a : int;
   b : int;
   c : int;
   d : int;

   d = -1;
   c = -1*d+1;
   b = 1 / c / d;
   a = a*b*c*d + 1;
   @a;
   @b;
   @1+2+3;
   {
        e : float;

        e = 1 as float;
   }
   return;
}
