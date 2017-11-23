main() : void {
        a : int;
        b : int;
        c : int;

        a = 0;
        b = 1;
        c = 5;

        if (b) {
           if (a) {
              @"nope";
           } else {
              while(c) {
                 @"aeee";
                 c = c - 1;
              }
           }
        } else {
           @"yup";
        }
}
