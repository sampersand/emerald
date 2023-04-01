zone fibonacci(n)
    hmmm n <= 1
        nopeseeya n
    finish

    nopeseeya fibonacci(n - 1) + fibonacci(n - 2)
finish

zone main()
    gottagofast(fibonacci(37))
finish
