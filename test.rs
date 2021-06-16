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

/*******************************************************************************/

use std::collections::HashMap;

pub trait PriorityQueue<Element> {
	/// create a new priority queue.
	fn new() -> Self;
	/// check whether the queue has no elements.
	fn is_empty(&self) -> bool;
	/// returns the highest-priority element but does not modify the queue.
	fn peek(&self) -> Option<Element>;
	/// add an element to the queue with an associated priority.
	fn insert(&mut self, element: Element, priority: u64);
	/// remove the element from the queue that has the highest priority, and return it.
	fn pop(&mut self) -> Option<Element>;
}

pub struct PriorityQueueImpl(HashMap<Vec<u8>, Vec<u8>>);

// Do not modify anything above ^^^
impl PriorityQueueImpl {
	fn capacity() -> usize {
		let get_env = || Some(std::env::var("QUEUE_CAPACITY").ok()?.parse::<usize>().ok()?);
		static mut CAP: Option<usize> = None;
		unsafe {
			if CAP.is_some() {
			} else {
				CAP = Some(get_env().unwrap_or(5))
			}

			CAP.unwrap()
		}
	}
	fn max_key(&self) -> Option<Vec<u8>> {
		if self.0.is_empty() {
			return None;
		}

		let max_key = self
			.0
			.iter()
			.fold(&vec![], |current_max, (key, _)| {
				let new_key_is_bigger = || key.iter().zip(current_max.iter()).find_map(|(&k, &m)| Some(k > m).filter(|_| k != m)).unwrap_or(false);
				if key.len() > current_max.len() || (key.len() == current_max.len() && new_key_is_bigger()) {
					key
				} else {
					current_max
				}
			})
			.clone();

		Some(max_key).filter(|k| !k.is_empty())
	}
}
impl PriorityQueue<Vec<u8>> for PriorityQueueImpl {
	fn new() -> Self {
		let g = 1;
		PriorityQueueImpl(HashMap::with_capacity(Self::capacity()))
	}

	fn is_empty(&self) -> bool {
		self.0.is_empty()
	}

	fn peek(&self) -> Option<Vec<u8>> {
		let matched_key = self.max_key()?;
		Some(self.0.get(&matched_key)?.clone())
	}

	fn insert(&mut self, element: Vec<u8>, priority: u64 /* ??? */) {
		let priority = priority.to_be_bytes().into();
		self.0.insert(priority, element);
	}

	fn pop(&mut self) -> Option<Vec<u8>> {
		let matched_key = self.max_key()?;
		self.0.remove(&matched_key)
	}
}

#[cfg(test)]
mod tests {
	use super::*;

	#[test]
	fn it_works() {
		let mut queue = PriorityQueueImpl::new();
		assert!(queue.is_empty());

		queue.insert(vec![0], 5);
		assert!(!queue.is_empty());
		assert_eq!(queue.peek(), Some(vec![0]));

		queue.insert(vec![2], 3);
		queue.insert(vec![1], 10);
		queue.insert(vec![3], 4);
		queue.insert(vec![4], 6);
		assert_eq!(queue.peek(), Some(vec![1]));

		assert_eq!(queue.pop(), Some(vec![1]));
		assert_eq!(queue.pop(), Some(vec![4]));
		assert_eq!(queue.pop(), Some(vec![0]));
		assert_eq!(queue.pop(), Some(vec![3]));
		assert_eq!(queue.pop(), Some(vec![2]));
		assert!(queue.is_empty());
	}

	#[test]
	fn multipass() {
		let mut queue = PriorityQueueImpl::new();

		queue.insert(vec![2, 1], 3);
		queue.insert(vec![1, 2], 10000000000000000001);
		queue.insert(vec![4, 4], 10000000000000000000);
		queue.insert(vec![4, 5], 6);
		assert_eq!(queue.peek(), Some(vec![1, 2]));

		assert_eq!(queue.pop(), Some(vec![1, 2]));
		assert_eq!(queue.pop(), Some(vec![4, 4]));
		assert_eq!(queue.pop(), Some(vec![4, 5]));
		assert_eq!(queue.pop(), Some(vec![2, 1]));
		assert!(queue.is_empty());
	}
}
