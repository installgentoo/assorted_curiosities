#include<assert.h>
#include<vector>
#include<algorithm>
#include<numeric>
#include<limits>
#include<random>

using namespace std;

struct node {
	node *next;
	void *data;
};

//1. tortoise and hare algorithm
bool contains_loops(node *head) {
	if(!head)
		return false;

	node *slow = head
			, *fast1 = head
			, *fast2;

	//check if 2x width frame bumps into slower one
	while((fast2 = fast1->next)
		  && (fast1 = fast2->next))
	{
		if(fast1 == slow
			|| fast2 == slow)
			return true;

		slow = slow->next;
	}

	return false;
}


//2. worst case scenario minimum number will be N+1, therefore we can mark every number from 1 to N+1 in a temp array, and take the first one unmarked
int min_pos(vector<int> v) {
	int N = v.size() + 1;
	vector<bool> filled(N, false);

	for(int i: v)
	{
		if(i > 0 && i < N)
			filled[i] = true;
	}

	for(int n=1; n<N; ++n)
	{
		if(!filled[n])
			return n;
	}

	return N;
}


//3. we can walk from both ends and build up sums for each position. then we just find the middle ground.
int split(vector<int> v) {
	vector<int> fwrd, back;

	{
		int sum = 0;
		std::transform(v.begin(), v.end(), back_inserter(fwrd), [&](auto v){ sum+=v; return sum; });
	}

	{
		int sum = 0;
		std::transform(v.rbegin(), v.rend(), back_inserter(back), [&](auto v){ sum+=v; return sum; });
		std::reverse(back.begin(), back.end());
	}

	int min_val = std::numeric_limits<int>::max()
			, mid;

	for(int i=1; i<v.size(); ++i)
	{
		int diff = std::abs(fwrd[i - 1] - back[i]);
		if(diff < min_val)
		{
			mid = i;
			min_val = diff;
		}
	}

	return mid;
}


//^ solutions are up there, code should be selfexplanatory

//V tests are down here

//build with ++14
int main()
{
	random_device r;
	mt19937 gen(r());


	//1.
	{
		auto add = [](auto n){
			n->next = new node{};
			return n->next;
		};

		auto head = new node{};
		auto *tmp = head;

		tmp = add(tmp);

		assert(!contains_loops(head));

		for(int i=0; i<8; ++i)
			tmp = add(tmp);

		assert(!contains_loops(head));

		tmp->next = head;

		assert(contains_loops(head));
	}

	//2.
	{
		uniform_int_distribution<int> d(-10000, 0);

		vector<int> noise(300);
		std::iota(noise.begin(), noise.end(), 3);
		std::generate_n(back_inserter(noise), 300, [&]{ return d(gen); });
		std::shuffle(noise.begin(), noise.end(), gen);

		int min = min_pos(noise);

		assert(min == 1);

		noise = { 3, 1, 2 };
		min = min_pos(noise);

		assert(min == 4);
	}

	//3.
	{
		uniform_int_distribution<int> d(-3, 10);

		vector<int> noise;
		std::generate_n(back_inserter(noise), 111, [&]{ return d(gen); });
		noise.emplace_back(100000);
		noise.emplace_back(100000);
		std::generate_n(back_inserter(noise), 232, [&]{ return d(gen); });

		int mid = split(noise);

		assert(mid == 112);

		noise = { 1, 2, 3, 4, 0, 4, 3, 1, 3 };
		mid = split(noise);

		assert(mid == 4);
	}

	return 0;
}
