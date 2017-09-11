main() : void {
   a, b : int;
   a = 1;
   b = 0;

   if (a) {
      if (b) {
         c : int;
         c = 3;
      }
   } else {
      c : int;
      c = 2;
   }

   if (b && a || c) {}
   else {}

   if (!b && !c) {
      if (a >= b) {
         if (a > b) {}
      } else {
        if (a == b) {}
        if (-a == -b) {}
        if (a < b) {}
        if (a <= b) {}
      }
   }
}
