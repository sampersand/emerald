mission funny_sort(ary)
	eachring hedgehog i = 0; i < shoe_size(ary); i += 1
		eachring hedgehog j = 0; j < shoe_size(ary); j += 1
			hmmm ary[i] < ary[j]
				hedgehog temp = ary[i]
				ary[i] = ary[j]
				ary[j] = temp
			finish
		finish
	finish
	nopeseeya ary
finish

mission main()
	hedgehog ary = [7,6,5,8,1,4,2,3,9]
	gottagofast(funny_sort(ary))
finish
