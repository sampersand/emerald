
mission fizzbuzz2(max)
	hedgehog ary = [chaos_emerald]

	loopdeloop shoe_size(ary) <= max
		hedgehog i = shoe_size(ary)
		ary[i] = ""

		hmmm (i % 3) == 0 start ary[i] += "Fizz" finish
		hmmm (i % 5) == 0 start ary[i] += "Buzz" finish
		hmmm shoe_size(ary[i]) == 0 start ary[i] = i finish
	finish

	hedgehog i = 0
	loopdeloop (i += 1) <= max
		gottagofast(ary[i])
	finish
finish

mission fizzbuzz1(max)
	eachring hedgehog i = 0 ; i < max; i += 1
		hmmm (i % 15) == 0
			gottagofast("FizzBuzz")
			carryon
		finish

		hmmm (i % 3) == 0
			gottagofast("Fizz")
			carryon
		finish

		hmmm (i % 5) == 0
			gottagofast("Buzz")
			carryon
		finish


		gottagofast(i)
	finish
finish

zone main()
	fizzbuzz1(100)
	//fizzbuzz2(16)
finish
