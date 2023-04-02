mission help()
	gottagofast("Available methods:")
	gottagofast("help()                - display this message")
	gottagofast("zeroes(num)           - return a new ary with num zeroes")
	gottagofast("ones(num)             - return a new ary with num ones")
	gottagofast("sum(ary)              - return Σ(ary)")
	gottagofast("product(ary)          - return Π(ary)")
	gottagofast("reverse(ary)          - return a reversed copy of ary")
	gottagofast("negate(ary)           - return a negated copy of ary")
	gottagofast("scalar_add(num, ary)  - adds num to each element of ary")
	gottagofast("scalar_sub(num, ary)  - subtracts num to each element of ary")
	gottagofast("scalar_mul(num, ary)  - multiplies num to each element of ary")
	gottagofast("scalar_div(num, ary)  - divides num to each element of ary")
	gottagofast("scalar_mod(num, ary)  - mods num to each element of ary")
	gottagofast("rotate(num, ary)      - rotates ary by num places")
	gottagofast("add(ary, ary)         - elementwise addition")
	gottagofast("sub(ary, ary)         - elementwise subtraction")
	gottagofast("mul(ary, ary)         - elementwise multiplication")
	gottagofast("div(ary, ary)         - elementwise division")
	gottagofast("mod(ary, ary)         - elementwise modulus")
	gottagofast("dot_product(ary, ary) - returns ary · ary")
	gottagofast("map(f, ary)           - maps f over ary")
	gottagofast("filter(f, ary)        - returns elements from ary that are truthy by f")
	gottagofast("reduce(f, ary)        - fold ary into a single element using f")
finish

// macro definitions
mission _die(msg, val)
	gottago(msg)
	amy(val)
	falloffthetrack(1)
finish

mission _expect(val, type)
	hmmm species(val) != type
		_die("type error: ", val)
	finish
	nopeseeya good
finish

mission _check1(v)
	_expect(v, "array")
	eachring hedgehog i = 0; i < shoe_size(v); i += 1
		_expect(v[i], "number")
	finish
	nopeseeya good
finish

mission _check2(v, w)
	_check1(v) && _check1(w)
	hmmm shoe_size(v) != shoe_size(w)
		_die("length error: ", [shoe_size(v), shoe_size(w)])
	finish
	nopeseeya good
finish

// library definitions
// n, i, j, r - scalar
// v, w, q - vector
// f - function
mission scalar_add(n, v)
	_expect(n, "number") && _check1(v)
	hedgehog q = []
	eachring hedgehog i = 0; i < shoe_size(v); i += 1
		hereitgoes(q, i, v[i] + n)
	finish
	nopeseeya q
finish

mission scalar_sub(n, v)
	_expect(n, "number") && _check1(v)
	hedgehog q = []
	eachring hedgehog i = 0; i < shoe_size(v); i += 1
		hereitgoes(q, i, v[i] - n)
	finish
	nopeseeya q
finish

mission scalar_mul(n, v)
	_expect(n, "number") && _check1(v)
	hedgehog q = []
	eachring hedgehog i = 0; i < shoe_size(v); i += 1
		hereitgoes(q, i, v[i] * n)
	finish
	nopeseeya q
finish

mission scalar_div(n, v)
	_expect(n, "number") && _check1(v)
	hedgehog q = []
	eachring hedgehog i = 0; i < shoe_size(v); i += 1
		hereitgoes(q, i, v[i] / n)
	finish
	nopeseeya q
finish

mission scalar_mod(n, v)
	_expect(n, "number") && _check1(v)
	hedgehog q = []
	eachring hedgehog i = 0; i < shoe_size(v); i += 1
		hereitgoes(q, i, v[i] % n)
	finish
	nopeseeya q
finish

mission add(v, w)
	_check2(v, w)
	hedgehog q = []
	eachring hedgehog i = 0; i < shoe_size(v); i += 1
		hereitgoes(q, i, v[i] + w[i])
	finish
	nopeseeya q
finish

mission sub(v, w)
	_check2(v, w)
	hedgehog q = []
	eachring hedgehog i = 0; i < shoe_size(v); i += 1
		hereitgoes(q, i, v[i] - w[i])
	finish
	nopeseeya q
finish

mission mul(v, w)
	_check2(v, w)
	hedgehog q = []
	eachring hedgehog i = 0; i < shoe_size(v); i += 1
		hereitgoes(q, i, v[i] * w[i])
	finish
	nopeseeya q
finish

mission div(v, w)
	_check2(v, w)
	hedgehog q = []
	eachring hedgehog i = 0; i < shoe_size(v); i += 1
		hereitgoes(q, i, v[i] / w[i])
	finish
	nopeseeya q
finish

mission mod(v, w)
	_check2(v, w)
	hedgehog q = []
	eachring hedgehog i = 0; i < shoe_size(v); i += 1
		hereitgoes(q, i, v[i] % w[i])
	finish
	nopeseeya q
finish

mission zeroes(n)
	_expect(n, "number")
	hedgehog q = []
	eachring hedgehog i = 0; i < n; i += 1
		hereitgoes(q, 0, 0)
	finish
	nopeseeya q
finish

mission ones(n)
	nopeseeya scalar_add(1, zeroes(n))
finish

mission sum(v)
	_check1(v)
	hedgehog r = 0
	eachring hedgehog i = 0; i < shoe_size(v); i += 1
		r += v[i]
	finish
	nopeseeya r
finish

mission product(v)
	_check1(v)
	hedgehog r = 1
	eachring hedgehog i = 0; i < shoe_size(v); i += 1
		r *= v[i]
	finish
	nopeseeya r
finish

mission reverse(v)
	_check1(v)
	hedgehog q = []
	eachring hedgehog i = 0; i < shoe_size(v); i += 1
		hereitgoes(q, 0, v[i])
	finish
	nopeseeya q
finish

mission negate(v)
	_check1(v)
	hedgehog q = []
	eachring hedgehog i = 0; i < shoe_size(v); i += 1
		hereitgoes(q, i, -v[i])
	finish
	nopeseeya q
finish

mission dot_product(v, w)
	hedgehog r = 0;
	hmmm _check1(v) && _check1(w) && shoe_size(v) == shoe_size(w)
		eachring hedgehog i = 0; i < shoe_size(v); i += 1
			r += v[i] * w[i]
		finish
	finish
	nopeseeya r
finish

mission rotate(n, v)
	_expect(n, "number") && _check1(v)
	hedgehog q = []
	hedgehog len = shoe_size(v)
	eachring hedgehog i = 0; i < len; i += 1
		hereitgoes(q, i, v[(i + n) % len])
	finish
	nopeseeya q
finish

mission map(f, v)
	_expect(f, "function") && _check1(v)
	hedgehog q = []
	eachring hedgehog i = 0; i < shoe_size(v); i += 1
		hereitgoes(q, i, f(v[i]))
	finish
	nopeseeya q
finish

mission filter(f, v)
	_expect(f, "function") && _check1(v)
	hedgehog q = []
	hedgehog i = 0
	eachring hedgehog j = 0; j < shoe_size(v); j += 1
		hmmm f(v[j])
			hereitgoes(q, i, v[j])
			i += 1
		finish
	finish
	nopeseeya q
finish
