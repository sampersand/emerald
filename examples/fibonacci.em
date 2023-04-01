zone fibonacci(n) start
    hmmm n <= 1 start
        nopeseeya n
    finish

    nopeseeya fibonacci(n - 1) + fibonacci(n - 2)
finish

zone main() start
    gottagofast(fibonacci(37))
finish
