#include <bits/stdc++.h>
using namespace std;

struct Node{
    int value;
    Node* left;
    Node* right;
    Node* parent;
    Node(int value, Node *parent = nullptr, Node *left = nullptr, Node *right = nullptr) : left(left), right(right), parent(parent), value(value) {}
};

class MinHeap{
    Node* root = nullptr;
    int size = 0;

    public:
        Node* percolateUp(Node* curr){
            if(!curr->parent) return curr;
            if(curr->value < curr->parent->value){
                swap(curr->value,curr->parent->value);
                percolateUp(curr->parent);
            }
            return curr;
        }
        Node* percolateDown(Node *curr)
        {
            if (!curr)
                return curr;

            int rightVal = curr->right ? curr->right->value : INT_MAX;
            int leftVal = curr->left ? curr->left->value : INT_MAX;
            bool isRightSmall = rightVal < leftVal; 
            if (isRightSmall && rightVal < curr->value )
            {
                swap(curr->value, curr->right->value);
                percolateDown(curr->right);
            }
            else if(!isRightSmall && leftVal < curr->value){
                swap(curr->value, curr->left->value);
                percolateDown(curr->left);
            }
            return curr;
        }
        int dequeue(){
            if(!root) return -1;
            int value = root->value;

            if(size == 1) {
                delete root;
                root= nullptr;
                size--;
                return value;
            }
            Node* toRemove= GetHole(size,false);
            int DanglingVal = toRemove->value;
            if(toRemove->parent->left==toRemove)
                 toRemove->parent->left= nullptr;
            else
                toRemove->parent->right = nullptr;
            delete toRemove;
            root->value = DanglingVal;

            percolateDown(root);
            size--;
            return value;

        }
        void insert(int val){
            if(!root) {
                Node* newNode = new Node(val);
                root = newNode;
                size++;
                return;
            }
            Node* newNode = GetHole(++size);
            newNode->value = val;
            percolateUp(newNode);
         

        } 
        Node* GetHole(int pos,bool insertDummy=true){
            
            int hole = pos;
            int msb = 0;
            int counter = hole;
            while(counter >>= 1){msb++;}
            msb = (1 << (msb-1));

            Node* temp= root;
            Node* newNode = new Node(0);
            while(temp){
                bool right = (hole&msb);
                
                if(right){
                    if(msb <= 1) {
                        if(insertDummy){
                            newNode->parent = temp; temp->right= newNode;
                        }
                            return temp->right;
                     }
                     temp=temp->right;
                    }
                else {
                    if (msb <= 1)
                    {
                        if(insertDummy){
                            newNode->parent = temp;
                            temp->left = newNode;
                        }
                        return temp->left;
                    }
                    temp=temp->left;
                }
                msb = (msb >> 1);
            }
            return nullptr;
        }

        void print(Node* root){
            if(!root) return;
            Node* temp = root;
            cout << temp->value << " ";
            print(temp->left);
            cout <<"-";
            print(temp->right);
        }

        Node* getRoot() const{return root;}
};


int main() {
    MinHeap heap={};
    heap.insert(7);
    heap.insert(6);
    heap.insert(5);
    heap.insert(10);
    heap.insert(4);
    heap.insert(3);
    heap.insert(2);
    heap.insert(1);
    int value = heap.dequeue();
    while(value != -1){
        cout << value << " ";
        value = heap.dequeue();
    }
   
    return 0;
}