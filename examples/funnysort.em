zone iCantBelieveItCanSort(ary)
	hedgehog i = 0
	loopdeloop i < shoe_size(ary)
		hedgehog j = 0
		loopdeloop j < shoe_size(ary)
			hmmm ary[i] < ary[j]
				hedgehog temp = ary[i]
				ary[i] = ary[j]
				ary[j] = temp
			finish
			j += 1
		finish
		i += 1
	finish
	nopeseeya ary
finish

zone main()
	hedgehog ary = [7,6,5,8,1,4,2,3,9]
	gottagofast(iCantBelieveItCanSort(ary))
finish
