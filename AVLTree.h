#ifndef AVLTREE_H_
#define AVLTREE_H_

#include <iostream>
#include <memory>
#include <utility>

#define max(a, b) (a > b ? a : b)
#define NodeArray Pair<Key, shared_ptr<Value>>**

using std::shared_ptr;

template <typename T1, typename T2>
struct Pair {
    T1 first;
    T2 second;
    Pair() {}
    Pair(T1 first, T2 second) : first(first), second(second) {}
};

template <typename Key, typename Value>
class AVLTree {
   public:
    class AVLTreeNode {
       public:
        Key key;
        shared_ptr<Value> data;
        AVLTreeNode* left = nullptr;
        AVLTreeNode* right = nullptr;
        AVLTreeNode* parent;         // final parent
        AVLTreeNode* inorderParent;  // parent before rolls

        int height = 0;

        AVLTreeNode(Key key, shared_ptr<Value> data, AVLTreeNode* parent, AVLTreeNode* inorderParent)
            : key(key), data(data), parent(parent), inorderParent(inorderParent) {}
    };

    class NodeAlreadyInserted {};
    class NodeDoesNotExist {};

   private:
    AVLTreeNode* root = nullptr;

    int height(AVLTreeNode* node) { return node == nullptr ? -1 : node->height; }

    void refreshHeight(AVLTreeNode* v) { v->height = max(height(v->left), height(v->right)) + 1; }

    // search data, if not found throws last visited node
    AVLTreeNode* search(AVLTreeNode* root, const Key key);

    void LL(AVLTreeNode* B, AVLTreeNode* A);
    void RR(AVLTreeNode* B, AVLTreeNode* A);

    bool evaluateRolls(AVLTreeNode* p, AVLTreeNode* v);

    void balanceUpwardsAtInsertion(AVLTreeNode* v);
    void balanceUpwardsAtRemoval(AVLTreeNode* v);

    void deleteTree(AVLTreeNode* root);

    void remove(AVLTreeNode* A);

    // utility functions for merging
    NodeArray toArray(int n);
    AVLTreeNode* fromArray(NodeArray array, int start, int end, AVLTreeNode* parent, int height);
    AVLTree fromArray(NodeArray array, int n);

    template <typename Function>
    void inorderTraversal(AVLTreeNode* root, Function func);

   public:
    AVLTree() {}
    ~AVLTree() { deleteTree(root); }

    AVLTree(AVLTree&& other) : root(other.root) { other.root = nullptr; }

    AVLTree& operator=(AVLTree&& other) {
        if (this != &other) {
            deleteTree(root);
            root = other.root;
            other.root = nullptr;
        }
        return *this;
    }

    friend void swap(AVLTree& t1, AVLTree& t2) { std::swap(t1.root, t2.root); }

    AVLTreeNode* search(const Key key) { return search(root, key); }

    AVLTreeNode* insert(Key key, shared_ptr<Value> data);

    void remove(Key key);

    AVLTree merge(int n1, AVLTree& tree2, int n2);

    template <typename Function>
    void inorderTraversal(Function func) { inorderTraversal(root, func); }
};

template <typename Key, typename Value>
typename AVLTree<Key, Value>::AVLTreeNode* AVLTree<Key, Value>::search(AVLTreeNode* root, const Key key) {
    if (root == nullptr)  // Empty tree
        throw root;

    if (key == root->key)  // Node found
        return root;

    if (key < root->key) {  // Smaller key
        if (root->left == nullptr)
            throw root;                  // Not in tree, throw latest node
        return search(root->left, key);  // Search in left subtree
    }

    if (root->right == nullptr)
        throw root;  // Not in tree, throw latest node

    return search(root->right, key);  // Search in right subtree
}

template <typename Key, typename Value>
void AVLTree<Key, Value>::LL(AVLTreeNode* B, AVLTreeNode* A) {
    B->left = A->right;  // moving Ar to Bl
    if (B->left != nullptr)
        B->left->parent = B;

    A->right = B;  // handle of new A position and parent
    A->parent = B->parent;

    if (B->parent == nullptr)  // handle of new B position and parent
        root = A;
    else if (B->parent->right == B)
        B->parent->right = A;
    else
        B->parent->left = A;
    B->parent = A;

    refreshHeight(B);
    refreshHeight(A);
}

template <typename Key, typename Value>
void AVLTree<Key, Value>::RR(AVLTreeNode* B, AVLTreeNode* A) {
    B->right = A->left;  // moving Al to Br
    if (B->right != nullptr)
        B->right->parent = B;

    A->left = B;  // handle of new A position and parent
    A->parent = B->parent;

    if (B->parent == nullptr)  // handle of new B position and parent
        root = A;
    else if (B->parent->right == B)
        B->parent->right = A;
    else
        B->parent->left = A;
    B->parent = A;

    refreshHeight(B);
    refreshHeight(A);
}

template <typename Key, typename Value>
bool AVLTree<Key, Value>::evaluateRolls(AVLTreeNode* p, AVLTreeNode* v) {
    int pBF = height(p->left) - height(p->right);
    int vBF = height(v->left) - height(v->right);

    if (pBF == 2) {
        if (vBF > -1)  // LL
            LL(p, p->left);
        else {  // LR
            RR(v, v->right);
            LL(p, p->left);
        }
        return true;
    }

    if (pBF == -2) {
        if (vBF < 1)  // RR
            RR(p, p->right);
        else {  // RL
            LL(v, v->left);
            RR(p, p->right);
        }
        return true;
    }

    return false;
}

template <typename Key, typename Value>
void AVLTree<Key, Value>::balanceUpwardsAtInsertion(AVLTreeNode* v) {
    AVLTreeNode* p;
    while (v != root) {
        p = v->parent;
        if (height(p) >= height(v) + 1)
            return;
        refreshHeight(p);
        if (evaluateRolls(p, v))
            return;
        v = p;
    }
}

template <typename Key, typename Value>
void AVLTree<Key, Value>::balanceUpwardsAtRemoval(AVLTreeNode* v) {
    if (v == nullptr)  // root was removed
        return;
    AVLTreeNode* p;
    while (v != root) {
        refreshHeight(v);
        p = v->parent;
        evaluateRolls(p, v);
        v = p;
    }
}

template <typename Key, typename Value>
void AVLTree<Key, Value>::deleteTree(AVLTreeNode* root) {
    if (root == nullptr)
        return;
    deleteTree(root->left);
    deleteTree(root->right);
    delete root;
}

template <typename Key, typename Value>
void AVLTree<Key, Value>::remove(AVLTreeNode* A) {
    AVLTreeNode* B = A->parent;

    if (A->right == nullptr && A->left == nullptr) {  // leaf

        if (B != nullptr) {  // update parent for removal
            if (B->left == A)
                B->left = nullptr;
            else
                B->right = nullptr;
        } else  // deleted the singular root
            root = nullptr;
        delete A;
    } else if (A->right != nullptr && A->left != nullptr) {  // 2 children
        AVLTreeNode* nextInorder = A->right;
        while (nextInorder->left != nullptr)
            nextInorder = nextInorder->left;

        A->key = nextInorder->key;
        A->data = nextInorder->data;

        remove(nextInorder);
    } else {  // 1 child
        AVLTreeNode* child = A->left != nullptr ? A->left : A->right;

        if (B == nullptr)  // A was root
            root = child;
        else if (B->left == A)  // A was left
            B->left = child;
        else  // A was right
            B->right = child;

        child->parent = B;
        delete A;
    }
    balanceUpwardsAtRemoval(B);
}

template <typename Key, typename Value>
NodeArray mergeArrays(NodeArray array1, int n1, NodeArray array2, int n2) {
    NodeArray out = new Pair<Key, shared_ptr<Value>>*[n1 + n2];
    int i = 0, j = 0, k = 0;
    while (i < n1 || j < n2)
        if (j == n2)  // second already finised
            out[k++] = array1[i++];
        else if (i == n1)  // first already finished
            out[k++] = array2[j++];
        else if (array1[i]->first < array2[j]->first)  // first's smaller
            out[k++] = array1[i++];
        else  // second's smaller
            out[k++] = array2[j++];

    delete[] array1;  // free arrays after usage
    delete[] array2;

    return out;
}

template <typename Key, typename Value>
NodeArray AVLTree<Key, Value>::toArray(int n) {
    NodeArray out = new Pair<Key, shared_ptr<Value>>*[n];
    int i = 0;
    inorderTraversal([&](AVLTreeNode* node) {  // fill array inorder
        out[i++] = new Pair<Key, shared_ptr<Value>>(node->key, node->data);
    });
    return out;
}

template <typename Key, typename Value>
typename AVLTree<Key, Value>::AVLTreeNode* AVLTree<Key, Value>::fromArray(NodeArray array, int start, int end, AVLTreeNode* parent, int height) {
    if (start > end)  // passed leaf
        return nullptr;
    int mid = (start + end) / 2;  // mid becomes subroot
    AVLTreeNode* root = new AVLTreeNode(array[mid]->first, array[mid]->second, parent, parent);
    root->height = height;                                            // pass height
    root->left = fromArray(array, start, mid - 1, root, height - 1);  // build left from left half
    root->right = fromArray(array, mid + 1, end, root, height - 1);   // build right from right half
    return root;
}

template <typename Key, typename Value>
AVLTree<Key, Value> AVLTree<Key, Value>::fromArray(NodeArray array, int n) {
    AVLTree out;
    int height = 0, _n = n;  // calculate expected height
    while (_n >>= 1)
        height++;
    out.root = fromArray(array, 0, n - 1, nullptr, height);

    for (int i = 0; i < n; i++)  // free pairs and array after usage
        delete array[i];
    delete[] array;

    return out;
}

template <typename Key, typename Value>
template <typename Function>
void AVLTree<Key, Value>::inorderTraversal(AVLTreeNode* root, Function func) {
    if (root == nullptr)
        return;
    inorderTraversal(root->left, func);
    func(root);
    inorderTraversal(root->right, func);
}

template <typename Key, typename Value>
typename AVLTree<Key, Value>::AVLTreeNode* AVLTree<Key, Value>::insert(Key key, shared_ptr<Value> data) {
    try {
        search(key);
        throw NodeAlreadyInserted();
    } catch (AVLTreeNode* parent) {
        AVLTreeNode* newNode = new AVLTreeNode(key, data, parent, parent);

        if (parent == nullptr)
            root = newNode;
        else if (key < parent->key)
            parent->left = newNode;
        else
            parent->right = newNode;

        balanceUpwardsAtInsertion(newNode);

        return newNode;
    }
}

template <typename Key, typename Value>
void AVLTree<Key, Value>::remove(Key key) {
    try {
        AVLTreeNode* toRemove = search(key);
        remove(toRemove);
    } catch (AVLTreeNode*) {
        throw NodeDoesNotExist();
    }
}

template <typename Key, typename Value>
AVLTree<Key, Value> AVLTree<Key, Value>::merge(int n1, AVLTree& tree2, int n2) {
    NodeArray array1 = toArray(n1);
    NodeArray array2 = tree2.toArray(n2);
    NodeArray array = mergeArrays(array1, n1, array2, n2);
    return fromArray(array, n1 + n2);
}

#endif  // AVLTREE_H_
