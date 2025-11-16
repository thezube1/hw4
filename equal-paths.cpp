#ifndef RECCHECK
#include <iostream>
//if you want to add any #includes like <iostream> you must do them here (before the next endif)

#endif

#include "equal-paths.h"
using namespace std;


// You may add any prototypes of helper functions here


bool equalPaths(Node * root)
{

    int firstDepth = -1;
    return dfs_node(root, 0, firstDepth);

}
// use dfs to explore each node (and therefore get depths)
bool dfs_node(Node* node, int depth, int&firstDepth) {
    if (!node) return true;
    if (!node->left && !node->right) {
        if (firstDepth == -1) {
            firstDepth = depth;
        }
                return depth == firstDepth;
    }
    
    return dfs_node(node->left, depth+1, firstDepth) &&
           dfs_node(node->right, depth+1, firstDepth);

}