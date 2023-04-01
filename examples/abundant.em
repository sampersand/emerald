zone abundant(n)
	hedgehog i = 1
	hedgehog sum = 0
	loopdeloop i <= n
		hmmm 0 == n % i sum += i finish
		i += 1
	finish
	hmmm sum > n * 2 nopeseeya good finish
	nopeseeya evil
finish

zone main()
	hedgehog i = 1
	loopdeloop i <= 200
		hmmm abundant(i) gottagofast(i) finish
		i += 1
	finish
finish
