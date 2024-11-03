#include <iostream>
#include "btree.h"

int main(int argc, char const *argv[])
{
    int order = 3;
    BplusTree bplustree2(order);

    bplustree2.loadFromFile("index.bin", order);
    //bplustree2.printTree(bplustree2.root);
    bplustree2.printTree(bplustree2.root);

    return 0;
}
