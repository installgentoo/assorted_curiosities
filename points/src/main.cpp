#include "../include/Parser.h"
#include "../include/Timer.h"
#include "../include/PointValidation.h"
#include <iostream>
#include <set>
#include <cmath>
#include <functional>
#include <thread>
#include <algorithm>

using std::pair;
using std::vector;
using std::multiset;
using std::function;
using std::thread;

int main(int argc, const char **argv)
{
	const auto pointSetFilename = "../point_set_small.txt"
			, solutionFilename = "../solution_small.txt";

	vector<Point> points = [&]{
		//measures the time in milliseconds
		const Timer timer("Point Loading");

		// reads the point data
		return Parser::readPoints(pointSetFilename);
	}();

	//empty vector to put your solution in
	vector<pair<Point, Point>> pointPairs;

	//i have a fast implementation of voronoi segmentation on gpu, but not for an arbitrary field size
	//i won't bother with yet another voronoi implementation, we all know how to google existing algos
	//instead i'll use a method i just came up with, because i find it amusing
	//i'll search amongst nearest points by x and y until their x and y distances become bigger than current shortest
	//following code is extremely simple so i'm not gonna comment on it
	using pointIter = vector<Point>::const_iterator;
	using SortedBy = multiset<pointIter, function<bool(pointIter, pointIter)>>;

	SortedBy by_x([](pointIter l, pointIter r){ return r->m_x > l->m_x; })
		   , by_y([](pointIter l, pointIter r){ return r->m_y > l->m_y; });

	vector<pair<SortedBy::const_iterator, SortedBy::const_iterator>> sorted_points;

	for(auto i=points.cbegin(); i<points.cend(); ++i)
		sorted_points.emplace_back(by_x.emplace(i), by_y.emplace(i));

	pointPairs.resize(sorted_points.size());

	const auto thread_num = thread::hardware_concurrency();

	const auto async_find_nearest = [thread_num, &sorted_points, &pointPairs, &by_x, &by_y](uint thread_id){
		for(size_t i=thread_id; i<sorted_points.size(); i+=thread_num)
		{
			const auto p = sorted_points[i];
			const auto point = *p.first;

			auto x_l = p.first
			   , x_r = x_l
			   , y_l = p.second
			   , y_r = y_l;

			bool x_l_is = true
			   , x_r_is = true
			   , y_l_is = true
			   , y_r_is = true;

			const auto dst_x = [](pointIter l, pointIter r){ return std::abs(l->m_x - r->m_x); };
			const auto dst_y = [](pointIter l, pointIter r){ return std::abs(l->m_y - r->m_y); };
			const auto dst = [](pointIter l, pointIter r){
				double x = l->m_x - r->m_x
					 , y = l->m_y - r->m_y;
				return std::sqrt(x * x + y * y);
			};

			double shortest = std::numeric_limits<double>::max();
			pointIter nearest = point;

			for(;x_l_is || x_r_is || y_l_is || y_r_is;)
			{
				x_l_is = (x_l_is && x_l != by_x.cbegin() && (dst_x(*(--x_l), point) < shortest)) ? true : false;
				x_r_is = (x_r_is && ++x_r != by_x.cend() && (dst_x(*x_r,     point) < shortest)) ? true : false;
				y_l_is = (y_l_is && y_l != by_y.cbegin() && (dst_y(*(--y_l), point) < shortest)) ? true : false;
				y_r_is = (y_r_is && ++y_r != by_y.cend() && (dst_y(*y_r,     point) < shortest)) ? true : false;

				const auto shorter = [&](pointIter tgt) {
					auto new_shortest = dst(tgt, point);
					if(shortest < new_shortest)
						return false;

					shortest = new_shortest;
					return true;
				};

				nearest = (x_l_is && shorter(*x_l)) ? *x_l : nearest;
				nearest = (x_r_is && shorter(*x_r)) ? *x_r : nearest;
				nearest = (y_l_is && shorter(*y_l)) ? *y_l : nearest;
				nearest = (y_r_is && shorter(*y_r)) ? *y_r : nearest;
			}

			pointPairs[i] = std::make_pair(*point, *nearest);
		}
	};

	vector<thread> threads;
	for(size_t i=0; i<thread_num; ++i)
		threads.emplace_back(thread(async_find_nearest, i));

	for(auto &t: threads)
		t.join();

	//validates your solution
	const std::string result = PointValidation::validatePoints(solutionFilename, pointPairs) ? "Correct" : "Wrong";
	std::cout << std::endl;
	std::cout << result << std::endl;

	int dummy;
	std::cin >> dummy;

	return 0;
}
