#ifndef AVLBST_H
#define AVLBST_H

#include <iostream>
#include <exception>
#include <cstdlib>
#include <cstdint>
#include <algorithm>
#include "bst.h"

struct KeyError { };

/**
* A special kind of node for an AVL tree, which adds the balance as a data member, plus
* other additional helper functions. You do NOT need to implement any functionality or
* add additional data members or helper functions.
*/
template <typename Key, typename Value>
class AVLNode : public Node<Key, Value>
{
public:
    // Constructor/destructor.
    AVLNode(const Key& key, const Value& value, AVLNode<Key, Value>* parent);
    virtual ~AVLNode();

    // Getter/setter for the node's height.
    int8_t getBalance () const;
    void setBalance (int8_t balance);
    void updateBalance(int8_t diff);

    // Getters for parent, left, and right. These need to be redefined since they
    // return pointers to AVLNodes - not plain Nodes. See the Node class in bst.h
    // for more information.
    virtual AVLNode<Key, Value>* getParent() const override;
    virtual AVLNode<Key, Value>* getLeft() const override;
    virtual AVLNode<Key, Value>* getRight() const override;

protected:
    int8_t balance_;    // effectively a signed char
};

/*
  -------------------------------------------------
  Begin implementations for the AVLNode class.
  -------------------------------------------------
*/

/**
* An explicit constructor to initialize the elements by calling the base class constructor
*/
template<class Key, class Value>
AVLNode<Key, Value>::AVLNode(const Key& key, const Value& value, AVLNode<Key, Value> *parent) :
    Node<Key, Value>(key, value, parent), balance_(0)
{

}

/**
* A destructor which does nothing.
*/
template<class Key, class Value>
AVLNode<Key, Value>::~AVLNode()
{

}

/**
* A getter for the balance of a AVLNode.
*/
template<class Key, class Value>
int8_t AVLNode<Key, Value>::getBalance() const
{
    return balance_;
}

/**
* A setter for the balance of a AVLNode.
*/
template<class Key, class Value>
void AVLNode<Key, Value>::setBalance(int8_t balance)
{
    balance_ = balance;
}

/**
* Adds diff to the balance of a AVLNode.
*/
template<class Key, class Value>
void AVLNode<Key, Value>::updateBalance(int8_t diff)
{
    balance_ += diff;
}

/**
* An overridden function for getting the parent since a static_cast is necessary to make sure
* that our node is a AVLNode.
*/
template<class Key, class Value>
AVLNode<Key, Value> *AVLNode<Key, Value>::getParent() const
{
    return static_cast<AVLNode<Key, Value>*>(this->parent_);
}

/**
* Overridden for the same reasons as above.
*/
template<class Key, class Value>
AVLNode<Key, Value> *AVLNode<Key, Value>::getLeft() const
{
    return static_cast<AVLNode<Key, Value>*>(this->left_);
}

/**
* Overridden for the same reasons as above.
*/
template<class Key, class Value>
AVLNode<Key, Value> *AVLNode<Key, Value>::getRight() const
{
    return static_cast<AVLNode<Key, Value>*>(this->right_);
}


/*
  -----------------------------------------------
  End implementations for the AVLNode class.
  -----------------------------------------------
*/


template <class Key, class Value>
class AVLTree : public BinarySearchTree<Key, Value>
{
public:
    virtual void insert (const std::pair<const Key, Value> &new_item); // TODO
    virtual void remove(const Key& key);  // TODO
protected:
    virtual void nodeSwap( AVLNode<Key,Value>* n1, AVLNode<Key,Value>* n2);

    // rotation helper functions
    AVLNode<Key, Value>* rotateLeft(AVLNode<Key, Value>* x);
    AVLNode<Key, Value>* rotateRight(AVLNode<Key, Value>* x);

    void rebalanceUpwards(AVLNode<Key, Value>* start);

    AVLNode<Key, Value>* rebalanceNode(AVLNode<Key, Value>* n);

    // helper utility functions (to reduce code re-use)
    static int heightNode(Node<Key, Value>* n);
    static int fixHeightAndBalance(AVLNode<Key, Value>* n);
    static int8_t balanceOf(AVLNode<Key, Value>* n);
    static AVLNode<Key, Value>* toAVL(Node<Key, Value>* n) { return static_cast<AVLNode<Key, Value>*>(n); }

};

/*
 * Recall: If key is already in the tree, you should 
 * overwrite the current value with the updated value.
 */
template<class Key, class Value>
void AVLTree<Key, Value>::insert (const std::pair<const Key, Value> &new_item)
{
    if (this->root_ == nullptr) {
        this->root_ = new AVLNode<Key, Value>(new_item.first, new_item.second, nullptr);
        return;
    }

    Node<Key, Value>* curr = this->root_;
    Node<Key, Value>* parent = nullptr;

    while (curr != nullptr) {
        parent = curr;
        if (new_item.first == curr->getItem().first) {
            // if key already exists rewrite it
            curr->setValue(new_item.second);
            return;
        }
        else if (new_item.first < curr->getItem().first) {
            curr = curr->getLeft();
        } else {
            curr = curr->getRight();
        }
    }

    AVLNode<Key, Value>* parentAVL = toAVL(parent);
    AVLNode<Key, Value>* newNode = new AVLNode<Key, Value>(new_item.first, new_item.second, parentAVL);

    if (new_item.first < parent->getItem().first) parentAVL->setLeft(newNode);
    else parentAVL->setRight(newNode);

    // rebalance the thang
    rebalanceUpwards(parentAVL);
}

/*
 * Recall: The writeup specifies that if a node has 2 children you
 * should swap with the predecessor and then remove.
 */
template<class Key, class Value>
void AVLTree<Key, Value>:: remove(const Key& key)
{
    AVLNode<Key, Value>* n = toAVL(this->internalFind(key));
    if (n == nullptr) return;

    // if there are two children swap with predecessor
    if (n->getLeft() != nullptr && n->getRight() != nullptr) {
        AVLNode<Key, Value>* pred = toAVL(BinarySearchTree<Key, Value>::predecessor(n));
        if (pred == nullptr) return; // just to be safe
        nodeSwap(n, pred);
    }

    // n has at most one child
    AVLNode<Key, Value>* parent = n->getParent();
    AVLNode<Key, Value>* child = (n->getLeft() != nullptr) ? n->getLeft() : n->getRight();

    if (child != nullptr) child->setParent(parent);

    if (parent == nullptr) {
        // remove root
        this->root_ = child;
    } else if (parent->getLeft() == n) {
        parent->setLeft(child);
    } else {
        parent->setRight(child);
    }

    delete n;

    rebalanceUpwards(parent);
}

template<class Key, class Value>
void AVLTree<Key, Value>::nodeSwap( AVLNode<Key,Value>* n1, AVLNode<Key,Value>* n2)
{
    BinarySearchTree<Key, Value>::nodeSwap(n1, n2);
    int8_t tempB = n1->getBalance();
    n1->setBalance(n2->getBalance());
    n2->setBalance(tempB);
}

// helper functions down below

template<class Key, class Value>
AVLNode<Key, Value>* AVLTree<Key, Value>::rotateLeft(AVLNode<Key, Value>* node)
{
    if (node == nullptr) return node;
    AVLNode<Key, Value>* rightChild = node->getRight();
    if (rightChild == nullptr) return node;

    AVLNode<Key, Value>* bottomLeftChild = rightChild->getLeft();
    AVLNode<Key, Value>* p = node->getParent();

    // link rightChild to node's parent
    rightChild->setParent(p);
    if (p == nullptr) this->root_ = rightChild;
    else if (p->getLeft() == node) p->setLeft(rightChild);
    else p->setRight(rightChild);

    // put node on rightChild's left
    rightChild->setLeft(node);
    node->setParent(rightChild);

    // move bottomLeftChild to node's right
    node->setRight(bottomLeftChild);
    if (bottomLeftChild != nullptr) bottomLeftChild->setParent(node);

    return rightChild;
}

template<class Key, class Value>
AVLNode<Key, Value>* AVLTree<Key, Value>::rotateRight(AVLNode<Key, Value>* node)
{
    if (node == nullptr) return node;
    AVLNode<Key, Value>* leftChild = node->getLeft();
    if (leftChild == nullptr) return node;

    AVLNode<Key, Value>* bottomRightChild = leftChild->getRight();
    AVLNode<Key, Value>* p = node->getParent();

    // link leftChild to node's parent
    leftChild->setParent(p);
    if (p == nullptr) this->root_ = leftChild;
    else if (p->getLeft() == node) p->setLeft(leftChild);
    else p->setRight(leftChild);

    // put node on leftChild's right
    leftChild->setRight(node);
    node->setParent(leftChild);

    // move bottomRightChild to node's left
    node->setLeft(bottomRightChild);
    if (bottomRightChild != nullptr) bottomRightChild->setParent(node);

    return leftChild;
}

template<class Key, class Value>
void AVLTree<Key, Value>::rebalanceUpwards(AVLNode<Key, Value>* start)
{
    AVLNode<Key, Value>* curr = start;
    while (curr != nullptr) {
        // update current node's balance from subtree heights
        curr->setBalance(balanceOf(curr));

        // if unbalanced, perform appropriate rotation(s)
        if (curr->getBalance() > 1 || curr->getBalance() < -1) {
            AVLNode<Key, Value>* newRoot = rebalanceNode(curr);
            // after rotations, recompute balances in the rotated subtree
            fixHeightAndBalance(newRoot);
            curr = newRoot->getParent();
        } else {
            curr = curr->getParent();
        }
    }
}

template<class Key, class Value>
AVLNode<Key, Value>* AVLTree<Key, Value>::rebalanceNode(AVLNode<Key, Value>* node)
{
    if (node == nullptr) return node;

    int b = node->getBalance(); // balance = h(left) - h(right)
    if (b > 1) {
        // the left side heavy
        AVLNode<Key, Value>* L = node->getLeft();
        if (balanceOf(L) < 0) {
            // LR case
            rotateLeft(L);
        }
        return rotateRight(node);
    }
    else if (b < -1) {
        // the right side's heavy
        AVLNode<Key, Value>* R = node->getRight();
        if (balanceOf(R) > 0) {
            // RL case
            rotateRight(R);
        }
        return rotateLeft(node);
    }
    return node;
}

template<class Key, class Value>
int AVLTree<Key, Value>::heightNode(Node<Key, Value>* node)
{
    if (node == nullptr) return 0;
    return 1 + std::max(heightNode(node->getLeft()), heightNode(node->getRight()));
}

template<class Key, class Value>
int AVLTree<Key, Value>::fixHeightAndBalance(AVLNode<Key, Value>* node)
{
    if (node == nullptr) return 0;
    int hl = fixHeightAndBalance(node->getLeft());
    int hr = fixHeightAndBalance(node->getRight());
    node->setBalance(static_cast<int8_t>(hl - hr));
    return 1 + (hl > hr ? hl : hr);
}

template<class Key, class Value>
int8_t AVLTree<Key, Value>::balanceOf(AVLNode<Key, Value>* node)
{
    if (node == nullptr) return 0;
    int hl = heightNode(node->getLeft());
    int hr = heightNode(node->getRight());
    return (hl - hr);
}

#endif
