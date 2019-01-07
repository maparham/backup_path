#ifndef UTILS_HPP_
#define UTILS_HPP_

// Program to print BFS traversal from a given
// source vertex. BFS(int s) traverses vertices
// reachable from s.
#include<iostream>
#include <list>
#include <execinfo.h> // for backtrace
#include <unistd.h>

using namespace std;
#define INF numeric_limits<int>::max()
typedef size_t Vertex;
typedef pair<size_t, size_t> Edge;
typedef list<Vertex> Set;
typedef vector<Vertex> Path;

void handler(int sig) {
	void* array[10];
	size_t size;

	// get void*'s for all entries on the stack
	size = backtrace(array, 10);

	// print out all the frames to stderr
	fprintf(stderr, "Error: signal %d:\n", sig);
	backtrace_symbols_fd(array, size, STDERR_FILENO);
	exit(1);
}

template<template<class, class > class C, class T, class A>
void print_vec(const C<T, A> &P) {
	std::ostream_iterator<T> out_it(std::cout, ",");
	std::copy(P.begin(), P.end(), out_it);
	cout << '\n';
}

void print_path(const Path &P) {
	print_vec(P);
}

// This class represents a directed graph using
// adjacency list representation
class Graph {
public:
	int V; // No. of vertices

	// Pointer to an array containing adjacency
	// lists
	vector<list<int>> adj;
	vector<int> pred;
	public:
	Graph(int V); // Constructor

	// function to add an edge to graph
	void addEdge(int v, int w);
	void removeEdge(int v, int w);
	size_t shortest_path(Vertex s, Vertex t);
	// prints BFS traversal from a given source s
	void BFS(int s);

	size_t size() {
		return V;
	}

	void print_dep() {
		for (int i = 0; i < V; ++i) {
			printf("%d: ", i);
			print_vec(adj[i]);
			printf("\n");
		}
	}
	bool hasArc(Vertex v, Vertex w) {
		return find(adj[v].begin(), adj[v].end(), w) != adj[v].end();
	}
	void clear() {
		adj.clear();
	}

};

void print_BP(vector<Path> BP) {
	for (Path P : BP) {
		print_path(P);
	}
}

Graph::Graph(int V) {
	this->V = V;
	adj.resize(V,{});
}

void Graph::addEdge(int v, int w) {
	adj[v].push_back(w); // Add w to v’s list.
}

void Graph::removeEdge(int v, int w) {
	adj[v].remove(w); // Add w to v’s list.
}

void Graph::BFS(int s) {
	pred.assign(V, -1);
	// Mark all the vertices as not visited
	vector<bool> visited(V);
	for (int i = 0; i < V; i++)
		visited[i] = false;

	// Create a queue for BFS
	list<int> queue;

	// Mark the current node as visited and enqueue it
	visited[s] = true;
	queue.push_back(s);

	// 'i' will be used to get all adjacent
	// vertices of a vertex
	list<int>::iterator i;

	while (!queue.empty()) {
		// Dequeue a vertex from queue and print it
		s = queue.front();
//		cout << s << ",";
		queue.pop_front();

		// Get all adjacent vertices of the dequeued
		// vertex s. If a adjacent has not been visited,
		// then mark it visited and enqueue it
//		printf("adj[%d]=", s);
//		print_vec(adj[s]);
		for (i = adj[s].begin(); i != adj[s].end(); ++i) {
			if (!visited[*i]) {
				visited[*i] = true;
				queue.push_back(*i);
				pred[*i] = s;
			}
		}
	}
//	print_vec(pred);
}
size_t Graph::shortest_path(Vertex s, Vertex t) {
	BFS(s);
	Path path;
	while (pred[t] > -1) {
		path.push_back(t);
		t = pred[t];
	}
//	path.push_back(s);
	return path.size() > 0 ? path.size() : INF;
}

class Powerset {
	size_t count1s(size_t v) {
		unsigned int c; // c accumulates the total bits set in v
		for (c = 0; v; v >>= 1) {
			c += v & 1;
		}
		return c;
	}
	void toSubset(size_t next, Path& s) {
		bitset<20> b(next);
		auto itr = U.cbegin();
		for (size_t i = 0; i < b.size(); ++i, ++itr) {
			if (b[i]) {
				s.push_back(*itr);
			}
		}
	}
	// find next k-combination
	// https://en.wikipedia.org/wiki/Combinatorial_number_system#Example
	void next_combination(unsigned long& x) // assume x has form x'01^a10^b in binary
			{
		unsigned long u = x & -x; // extract rightmost bit 1; u =  0'00^a10^b
		unsigned long v = u + x; // set last non-trailing bit 0, and clear to the right; v=x'10^a00^b
		if (v == 0) // then overflow in v, or x==0
				{
			//return false; // signal that next k-combination cannot be represented
		}
		x = v + (((v ^ x) / u) >> 2); // v^x = 0'11^a10^b, (v^x)/u = 0'0^b1^{a+2}, and x ← x'100^b1^a
	}
#if 1
	bool next_subset(unsigned long& x) { // in the ascending order of set cardinality
		size_t s = count1s(x); // current cardinality
		next_combination(x);
		if (x < Pset) { // next exists?
			return true;
		} else if (++s <= N) { // next cardinality
			x = (1 << s) - 1;
			return true;
		}
		return false;
	}
#else
	bool next_subset(unsigned long& x) { // in binary order
		if (++x < Pset) {
			return true;
		}
		return false;
	}
#endif
public:
	Powerset(Set U) :
			U(U), N(U.size()), Pset(1 << N) {
	}
	const Set U;
	const size_t N;
	const size_t Pset;

	void forEach(function<bool(const Path&)> func) {
		size_t next = 1;
		Path selection;
		bool stop = false;
		do {
			selection.clear();
			toSubset(next, selection);
//			printf("next=%lu, selection.size=%lu;\n", next,selection.size());
			do {
				stop = func(selection);
				if (stop) {
					return;
				}
//						std::ostream_iterator<size_t> out_it(std::cout, ",");
//						std::copy(selection.begin(), selection.end(), out_it);
//						cout << '\n';
			} while (next_permutation(selection.begin(),
					selection.end()));
		} while (next_subset(next));
	}
};

size_t factorial(size_t n)
		{
	return (n == 1 || n == 0) ? 1 : factorial(n - 1) * n;
}

size_t choose(size_t n, size_t k) {
	return factorial(n) / (factorial(k) * factorial(n - k));
}

size_t sum(size_t a, size_t b) {
	if (a > b) {
		return 0;
	}
	auto s = a;
	while (a++ < b) {
		s += a;
	}
	return s;
}
clock_t duration_ms(const clock_t d) {
	return d * 1000 / CLOCKS_PER_SEC;
}

#endif /* UTILS_HPP_ */
