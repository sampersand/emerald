zone abundant(n)
	hedgehog sum = 0
	eachring hedgehog i = 1; i <= n; i += 1
		hmmm 0 == n % i sum += i finish
	finish
	hmmm sum > n * 2 nopeseeya good finish
	nopeseeya evil
finish

zone main()
	eachring hedgehog i = 0; i <= 200; i += 1
		hmmm abundant(i) gottagofast(i) finish
	finish
finish
