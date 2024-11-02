#include <iostream>
#include "btree.h"

int main(int argc, char const *argv[])
{
    int order = 3;
    BplusTree bplustree2(order);

    int id = 5;
    void* blockPtr = reinterpret_cast<void*>(0x1000);

    bplustree2.loadFromFile("index.bin", order);
    cout << "Loaded" << endl;
    bplustree2.printTree(bplustree2.root, 3);
    if (bplustree2.find(id, blockPtr)) {
        cout << "Found" << endl;
    } else {
        cout << "Not found" << endl;
    }

    return 0;
}
