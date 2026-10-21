#pragma once
#include <bits/stdc++.h>
using namespace std;

// ── Node ─────────────────────────────────────────────────────────────────────
struct Node
{
    int frequency;
    uint16_t data;
    Node *left  = nullptr;
    Node *right = nullptr;
    Node(int frequency, uint16_t data = '*') : frequency(frequency), data(data) {}
};

// ── PriorityQueue (min-heap of Node*) ────────────────────────────────────────
class PriorityQueue
{
    static const int CAP = 5;
    Node **queue   = nullptr;
    int   size     = 0;
    int   capacity = CAP;

    void shrink();
    void extend();

public:
    PriorityQueue();
    ~PriorityQueue();

    void enqueue(Node *node);
    Node *dequeue();
    void percolateUp(int index);
    void percolateDown(int index);
    void print() const;
    int  getSize() const { return size; }
};
