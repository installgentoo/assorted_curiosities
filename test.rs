use hashbrown::HashMap;
use rug::Integer;

macro_rules! UnsafeOnce {
	($t: ty, $b: block) => {{
		static mut S: Option<$t> = None;
		unsafe {
			if S.is_some() {
			} else {
				S = Some($b);
			}
			S.as_mut()
		}
		.unwrap()
	}};
}

fn main() {
	let fizzbuzz = |to| {
		for i in 1..=to {
			let l = match (i % 3, i % 5) {
				(0, 0) => "FizzBuzz".into(),
				(0, _) => "Fizz".into(),
				(_, 0) => "Buzz".into(),
				(_, _) => i.to_string(),
			};
			println!("{}", l);
		}
	};

	fizzbuzz(100);

	//O(2^n), O(n)

	fn fibonacci_rec(n: u32) -> Integer {
		if n == 0 || n == 1 {
			return Integer::from(n);
		}
		let cache = UnsafeOnce!(HashMap<u32, Integer>, { HashMap::new() });
		if let Some(r) = cache.get(&n) {
			return r.clone();
		}
		let r = fibonacci_rec(n - 1) + fibonacci_rec(n - 2);
		cache.insert(n, r.clone());
		r
	}

	println!("{}", fibonacci_rec(250));

	//O(n)

	fn fibonacci_iter(n: u32) -> Integer {
		if n == 0 || n == 1 {
			return Integer::from(n);
		}
		let mut cache = (Integer::from(0), Integer::from(1));
		for _ in 2..=n {
			let (n_1, n_2) = cache;
			cache = (n_2.clone(), n_1 + n_2);
		}
		cache.1
	}

	println!("{}", fibonacci_iter(250));

	struct Node<T> {
		first_child: Option<Box<Node<T>>>,
		first_next: Option<Box<Node<T>>>,
		val: T,
	}

	fn traverse<T>(base: &Node<T>) {
		if let Some(child) = &base.first_child {
			traverse(&child);
		}
		if let Some(next) = &base.first_next {
			traverse(&next);
		}
	}
}
