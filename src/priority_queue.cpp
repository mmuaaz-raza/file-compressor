#include "../include/priority_queue.hpp"


void PriorityQueue::shrink()
{
    int new_capacity = max(CAP, capacity - CAP);
    if (new_capacity == capacity) return;
    Node **tempQueue = new Node *[new_capacity];
    capacity = new_capacity;
    for (int i = 0; i < size; i++)
        tempQueue[i] = queue[i];
    delete[] queue;
    queue = tempQueue;
}

void PriorityQueue::extend()
{
    Node **tempQueue = new Node *[size + CAP];
    capacity = size + CAP;
    for (int i = 0; i < size; i++)
        tempQueue[i] = queue[i];
    delete[] queue;
    queue = tempQueue;
}

PriorityQueue::PriorityQueue()
{
    queue = new Node *[CAP];
}

PriorityQueue::~PriorityQueue()
{
    delete[] queue;
}

void PriorityQueue::enqueue(Node *node)
{
    if (size >= capacity) extend();
    queue[size] = node;
    percolateUp(size);
    size++;
}

void PriorityQueue::percolateUp(int index)
{
    int parent = ((index + 1) / 2) - 1;
    if (parent < 0) return;
    if (queue[index]->frequency < queue[parent]->frequency)
    {
        swap(queue[index], queue[parent]);
        percolateUp(parent);
    }
}

void PriorityQueue::percolateDown(int index)
{
    int lefti   = (index * 2) + 1;
    int righti  = (index * 2) + 2;
    int smallest = index;

    if (lefti < size && queue[lefti]->frequency < queue[smallest]->frequency)
        smallest = lefti;
    if (righti < size && queue[righti]->frequency < queue[smallest]->frequency)
        smallest = righti;

    if (index != smallest)
    {
        swap(queue[index], queue[smallest]);
        percolateDown(smallest);
    }
}

void PriorityQueue::print() const
{
    for (int i = 0; i < size; i++)
        cout << queue[i] << " ";
    cout << "\n";
}

Node *PriorityQueue::dequeue()
{
    if (!size) return nullptr;
    Node *value = queue[0];
    queue[0] = queue[--size];
    if (size <= 0) return value;
    percolateDown(0);
    if ((capacity - size) >= (int(CAP / 2) + CAP)) shrink();
    return value;
}
