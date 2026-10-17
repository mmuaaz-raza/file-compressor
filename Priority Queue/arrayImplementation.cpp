#include <bits/stdc++.h>
// #include "../huffman.cpp"
using namespace std;
const int CAP = 5;

struct Node {
    int frequency;
    char data;
    Node *left = nullptr;
    Node *right = nullptr;
    Node(int frequency, char data='*') : frequency(frequency), data(data) {}
};

class PriorityQueue{
    Node** queue = nullptr;
    int size = 0 ;
    int capacity = CAP;
    void shrink()
    {
        int new_capacity = max(CAP, capacity - CAP);
        if(new_capacity == capacity) return;
        Node** tempQueue = new Node*[new_capacity];
        capacity = new_capacity;
        for (int i = 0; i < size; i++)
        {
            tempQueue[i] = queue[i];
        }
        delete[] queue;
        queue = tempQueue;
    }
    void extend()
    {
        Node** tempQueue = new Node*[size + CAP];
        capacity = size + CAP;
        for (int i = 0; i < size; i++)
        {
            tempQueue[i] = queue[i];
        }
        delete[] queue;
        queue = tempQueue;
    }

    public:
    PriorityQueue(){
        queue = new Node*[CAP];
    }
    void enqueue(Node* node)
    {
        if (size >= capacity)
        extend();
        queue[size] = node;
        percolateUp(size);
        size++;
        return;
    }

    void percolateUp(int index){
        int parent = ((index+1)/2) - 1;
        if( parent < 0) return;
        if(queue[index]->frequency < queue[parent]->frequency){
            swap(queue[index],queue[parent]);
            percolateUp(parent);
        }
        return;
    }
    void percolateDown(int index)
    {
        int lefti = (index * 2) + 1;
        int righti = (index * 2) + 2;

        int smallest = index;

        if (lefti < size && queue[lefti]->frequency < queue[smallest]->frequency)
        {
            smallest = lefti;
        }
        if (righti < size && queue[righti]->frequency < queue[smallest]->frequency)
        {
            smallest = righti;
        }

        if(index!=smallest){
            swap(queue[index], queue[smallest]);
            percolateDown(smallest);
        }
    }

  
    void print(){
        for (int i = 0; i < size; i++)
        {
            cout << queue[i] << " ";
        }
        cout << "\n";
        
    }
    
    Node* dequeue(){
        if(!size) return nullptr;
        Node* value = queue[0];
        queue[0] = queue[--size];
        if(size<=0) return value;
        percolateDown(0);
        if((capacity-size) >= (int(CAP/2)+CAP) ) shrink();
        return value;

    }
    int getSize() const {return size;}
    ~PriorityQueue(){
        delete[] queue;
    }
};

// int main() {
//    PriorityQueue queue;
//    for (int i =100; i >= 0; i--)
//    {
//         queue.insert(new Node(i));
//    }
   
//    Node* value = queue.dequeue();
//    while (value->frequency != -1)
//    {
//        cout << value->frequency << " ";
//        value = queue.dequeue();
//    }

//    return 0;
// }