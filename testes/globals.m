# it should be OK

global1 : float[];

foo() : int
{
   return -1;
}

bar() : float
{
   return 1.1234567;
}

global2 : int[ ];

foobar() : float []
{
   global1 = new float[123*12];
   return global1;
}

main() : void
{
   local1 : int;
   return;
}

global3 : int;
