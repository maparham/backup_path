#ifndef UTILS_HPP_
#define UTILS_HPP_

// Program to print BFS traversal from a given
// source vertex. BFS(int s) traverses vertices
// reachable from s.
#include<iostream>
#include <list>

using namespace std;
#define INF numeric_limits<int>::max()
typedef size_t Vertex;
typedef list<Vertex> Set;
typedef vector<Vertex> Path;

template<template<class, class > class C, class T, class A>
void print_vec(C<T, A> &P) {
	std::ostream_iterator<T> out_it(std::cout, ",");
	std::copy(P.begin(), P.end(), out_it);
	cout << '\n';
}

void print_path(Path &P) {
	print_vec(P);
}

// This class represents a directed graph using
// adjacency list representation
class Graph {
	int V; // No. of vertices

	// Pointer to an array containing adjacency
	// lists
	list<int> *adj;
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
};

Graph::Graph(int V) {
	this->V = V;
	adj = new list<int> [V];
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
	bool *visited = new bool[V];
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
	vector<int> path;
	while (pred[t] > -1) {
		path.push_back(t);
		t = pred[t];
	}
//	path.push_back(s);
	return path.size() > 0 ? path.size() : INF;
}

class Powerset {
public:
	Powerset(Set U) :
			U(U), Pset(1 << U.size()) {
	}
	const Set U;
	const size_t Pset;

	template<class BitSet>
	Set toSubset(BitSet b) {
		Set s;
		auto itr = U.cbegin();
		for (size_t i = 0; i < b.size(); ++i, ++itr) {
			if (b[i]) {
				s.push_back(*itr);
			}
		}
		return s;
	}

	void forEach(function<void(const Set&)> func) {
		const size_t N = U.size();
		for (size_t s = 1; s <= N; s++) {
			for (size_t i = 1; i < Pset; i++) {
				bitset<7> b(i);
				if (b.count() == s) { // a subset of size s
					Set selection = toSubset(b);
					do {
						func(selection);
//						std::ostream_iterator<size_t> out_it(std::cout, ",");
//						std::copy(selection.begin(), selection.end(), out_it);
//						cout << '\n';
					} while (next_permutation(selection.begin(),
							selection.end()));
				}
			}
		}
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
#endif /* UTILS_HPP_ */
