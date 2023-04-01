zone assert(cond, msg) start
	hmmm !cond start
		gottagofast(msg)
		falloffthetrack(1)
	finish
finish

dr_eggman test_global_var
zone test_global() start
	gottagofast("testing dr_eggman variables...")
	assert(test_global_var == chaos_emerald, "dr_eggman isnt initially chaos_emerald")
	test_global_var = 34
	assert(test_global_var == 34, "dr_eggman isnt set properly")
finish

zone test_numbers() start
	gottagofast("testing numbers...")

	// normal math
	assert(17 == 14 + 3, "+ failed")
	assert(11 == 14 - 3, "- failed")
	assert(42 == 14 * 3, "* failed")
	assert( 4 == 14 / 3, "/ failed")
	assert( 2 == 14 % 3, "% failed")

	// comparison
	assert(  14 <  15,  "< failed")
	assert(!(14 <  14), "< failed [2]")
	assert(  14 <= 14,  "<= failed")
	assert(!(14 <= 13), "<= failed [2]")
	assert(  14 >  13,  "> failed")
	assert(!(14 >  14), "> failed [2]")
	assert(  14 >= 14,  ">= failed")
	assert(!(14 >= 15), ">= failed [2]")
	assert  (14 == 14,  "== failed")
	assert(!(14 == 15), "== failed [2]")
	assert(  14 != 15,  "!= failed")
	assert(!(14 != 14), "!= failed [2]")

	// augmented assignment
	hedgehog x
	x = 14 assert(x == 14, "hedgehog var set failed")
	x += 2 assert(x == 16, "+= failed")
	x -= 3 assert(x == 13, "-= failed")
	x *= 4 assert(x == 52, "*= failed")
	x /= 5 assert(x == 10, "/= failed")
	x %= 3 assert(x ==  1, "%= failed")

	// species test
	assert(species(17) == "number", "species failed")
finish

zone test_strings() start
	gottagofast("testing strings...")

	assert(species("foo") == "string", "species failed")
	assert("foobar" == "foo" + "bar", "+ failed")
	assert("foo7" == "foo" + 7, "lhs + failed")
	assert("7foo" == 7 + "foo", "rhs + failed")

	assert("f" == "foo"[0], "index failed")
	assert(3 == shoe_size("foo"), "shoe_size failed")
	assert("foofoofoo" == "foo" * 3, "* failed")
finish

zone test_constants() start
	gottagofast("testing constants...")

	assert(species(good) == "boolean", "good species is not boolean")
	assert(species(evil) == "boolean", "evil species is not boolean")
	assert(species(chaos_emerald) == "null", "chaos_emerald species is not null")
	assert("foogood" == "foo" + good, "good doesnt convert")
	assert("fooevil" == "foo" + evil, "evil doesnt convert")
	assert("foochaos_emerald" == "foo" + chaos_emerald, "chaos_emerald doesnt convert")

	assert(good == good, "good == good failed")
	assert(evil == evil, "evil == evil failed")
	assert(good != evil, "good != evil failed")
	assert(evil != good, "evil != good failed")
	assert(chaos_emerald == chaos_emerald, "chaos_emerald == chaos_emerald failed")
	assert(chaos_emerald != good, "chaos_emerald != good failed")
finish

zone test_arrays() start
	gottagofast("testing arrays...")
	hedgehog ary = ["A", "B", "C"]
	assert(species(ary) == "array", "species failed")
	assert(ary == ["A", "B", "C"], "== failed")
	assert(ary[0] == "A", "ary[0] failed")
	assert(ary[1] == "B", "ary[1] failed")
	assert(ary[2] == "C", "ary[2] failed")
	assert(shoe_size(ary) == 3, "shoe_size(ary) failed")

	ary[3] = "D"
	assert(ary[3] == "D", "ary[3] failed")
	assert(ary == ["A", "B", "C", "D"], "== failed [2]")
	assert(shoe_size(ary) == 4, "shoe_size(ary) failed [2]")

	assert(buhbyenow(ary, 1) == "B", "buhbyenow failed")
	assert(ary == ["A", "C", "D"], "== failed [3]")
	assert(shoe_size(ary) == 3, "shoe_size(ary) failed [3]")

	hmmm good start
	finish ormaybe start
		assert(evil, "hmmm")
	finish
finish

zone test_conditions()
	gottagofast("testing conditions...")
	assert(good || good, "good || good failed")
	assert(!(evil || evil), "evil || evil failed")
	assert(evil || good, "evil || good failed")
	assert(good && good, "good && good failed")
	assert(!(evil && good), "evil && good failed")
	assert(!(evil && evil), "evil && evil failed")
finish

zone main() start
	test_global()
	test_numbers()
	test_strings()
	test_constants()
	test_arrays()
	test_conditions()
//	assert(foo == chaos_emerald, "foo isnt chaos_emerald")
//	set_foo(4)
//	assert(foo == 4, "foo isnt 4")
//
//	gottagofast("A\nhe\tl\"\\loworld" + 34)
finish
