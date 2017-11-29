foo(a : int) : int {
        a = 10;
        return a;
}

main(b : int) : void {
    a : int;
    foo(a);
}
