#include <iostream>
#include <bitset>
#include <algorithm>
#include <vector>
#include <iterator>     // std::ostream_iterator
#include <functional>

#include "utils.hpp"
#include <fstream>
#include "ILP.hpp"
#include "bpa.hpp"

using namespace std;

#define GREEDY 1
#if GREEDY
// exhaustive search
int main() {
	int N = 80;
	clock_t t = clock();
	vector<Edge> links;
	for (int i = 0; i < N; ++i) {
		for (int j = i + 1; j < N; ++j) {
			links.push_back(Edge(i, j));
		}
	}
	BPA bpa(N, links.size());
//	size_t x = factorial(links.size());
//	do {
//	bpa.search(links);
	bpa.greedy();
//		if (--x % 1000 == 0) {
//			printf("x=%lu\n", x);
//		}
//	}while (next_permutation(links.begin(), links.end()));
	clock_t elapsed_secs = double(clock() - t) / CLOCKS_PER_SEC;
	printf("elapsed time=%lu sec\n", elapsed_secs);
}
#else
int main() {
	signal(SIGSEGV, handler); // install our handler
	clock_t t = clock();

	const int N = 5, R = N - 2;
	vector<Edge> links;
	for (int i = 0; i < N; ++i) {
		for (int j = i + 1; j < N; ++j) {
			//			const bool regular3 =  (j-i == 3 || j-i == 1 || j-i == N-1);
			const bool _4regular = (j - i) <= 2 || (j - i) >= N - 2;
			if (1 || _4regular) {
				//				printf("(i, j)=%d,%d\n", i, j); r
				links.push_back(Edge(i, j));
				links.push_back(Edge(j, i));
			}
		}
	}

	/*const int N = 8, R = 2;
	 vector<Edge> links;
	 links.push_back(Edge(0, 1));
	 links.push_back(Edge(0, 2));
	 links.push_back(Edge(0, 4));
	 links.push_back(Edge(1, 3));
	 links.push_back(Edge(1, 5));
	 links.push_back(Edge(2, 3));
	 links.push_back(Edge(2, 6));
	 links.push_back(Edge(3, 7));
	 links.push_back(Edge(4, 5));
	 links.push_back(Edge(4, 6));
	 links.push_back(Edge(5, 7));
	 links.push_back(Edge(6, 7));
	 for (Edge l : links) {
	 Edge l_r(l.second, l.first);
	 links.push_back(l_r);
	 }*/

	BPA_MIP mip(N, links, R);
	vector<Path> scheme = mip.run();
	print_BP(scheme);

	clock_t elapsed_secs = double(clock() - t) / CLOCKS_PER_SEC;
	printf("elapsed time=%lu sec\n", elapsed_secs);
	/*
	 size_t count = 0, all = factorial(15);
	 BPA bpa(N, links.size());
	 do {
	 bool worked = true;
	 for (const Path bp : scheme) {
	 if (bp.size() < 4) {
	 continue;
	 }
	 for (int i = 1; i < bp.size() - 1; ++i) {
	 Path P = bp;
	 //				printf("erasing %lu\n", *(P.begin() + i));
	 P.erase(P.begin() + i);
	 bpa.updateDependencies(P, false);
	 if (bpa.isFeasible(bp) && !bpa.hasShortCycle(bp)) {
	 worked = false;
	 break;
	 }
	 bpa.updateDependencies(P, true);
	 }
	 if (!worked) {
	 break;
	 }
	 }
	 if (worked) {
	 printf("greedy ordering exists!\n");
	 print_BP(scheme);
	 return 0;
	 }
	 if (++count % 1000 == 0) {
	 printf("%f\n", (float) count / (float)all);
	 }
	 bpa.clear();
	 } while (next_permutation(scheme.begin(), scheme.end()));
	 */
}
#endif
