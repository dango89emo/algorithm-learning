#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <stdexcept>

const int MAX_CHILD = 5;
const int HALF_CHILD = (MAX_CHILD + 1) / 2;

using KEY = int;
using DATA = int;

class Node {
public:
    virtual ~Node() = default;
    virtual bool isLeaf() const = 0;
};

class InternalNode : public Node {
public:
    int nchildren;
    std::vector<std::unique_ptr<Node>> child;
    std::vector<KEY> low;

    InternalNode() : nchildren(0), child(MAX_CHILD), low(MAX_CHILD) {}
    bool isLeaf() const override { return false; }
};

class LeafNode : public Node {
public:
    KEY leaf_key;
    DATA your_data;

    LeafNode(KEY key) : leaf_key(key), your_data(0) {}
    bool isLeaf() const override { return true; }
};

class BTree {
private:
    std::unique_ptr<Node> root;

    int locate_subtree(const InternalNode* p, KEY key) const {
        for (int i = p->nchildren - 1; i > 0; i--)
            if (key >= p->low[i])
                return i;
        return 0;
    }

    Node* search_aux(Node* node, KEY key) const {
        if (node->isLeaf()) {
            auto leaf = static_cast<LeafNode*>(node);
            return (leaf->leaf_key == key) ? leaf : nullptr;
        } else {
            auto internal = static_cast<InternalNode*>(node);
            int i = locate_subtree(internal, key);
            return search_aux(internal->child[i].get(), key);
        }
    }

    std::unique_ptr<Node> insert_aux(std::unique_ptr<Node>& node, KEY key, KEY& lowest) {
        if (node->isLeaf()) {
            auto leaf = static_cast<LeafNode*>(node.get());
            if (leaf->leaf_key == key)
                return nullptr;
            auto new_leaf = std::make_unique<LeafNode>(key);
            if (key < leaf->leaf_key) {
                lowest = leaf->leaf_key;
                return std::move(node);
            } else {
                lowest = key;
                return std::move(new_leaf);
            }
        } else {
            auto internal = static_cast<InternalNode*>(node.get());
            int pos = locate_subtree(internal, key);
            KEY xlow;
            auto xnode = insert_aux(internal->child[pos], key, xlow);
            if (!xnode)
                return nullptr;
            
            if (internal->nchildren < MAX_CHILD) {
                for (int i = internal->nchildren - 1; i > pos; i--) {
                    internal->child[i + 1] = std::move(internal->child[i]);
                    internal->low[i + 1] = internal->low[i];
                }
                internal->child[pos + 1] = std::move(xnode);
                internal->low[pos + 1] = xlow;
                internal->nchildren++;
                return nullptr;
            } else {
                auto new_node = std::make_unique<InternalNode>();
                if (pos < HALF_CHILD - 1) {
                    for (int i = HALF_CHILD - 1, j = 0; i < MAX_CHILD; i++, j++) {
                        new_node->child[j] = std::move(internal->child[i]);
                        new_node->low[j] = internal->low[i];
                    }
                    for (int i = HALF_CHILD - 2; i > pos; i--) {
                        internal->child[i + 1] = std::move(internal->child[i]);
                        internal->low[i + 1] = internal->low[i];
                    }
                    internal->child[pos + 1] = std::move(xnode);
                    internal->low[pos + 1] = xlow;
                } else {
                    int j = MAX_CHILD - HALF_CHILD;
                    for (int i = MAX_CHILD - 1; i >= HALF_CHILD; i--) {
                        if (i == pos) {
                            new_node->child[j] = std::move(xnode);
                            new_node->low[j--] = xlow;
                        }
                        new_node->child[j] = std::move(internal->child[i]);
                        new_node->low[j--] = internal->low[i];
                    }
                    if (pos < HALF_CHILD) {
                        new_node->child[0] = std::move(xnode);
                        new_node->low[0] = xlow;
                    }
                }
                internal->nchildren = HALF_CHILD;
                new_node->nchildren = MAX_CHILD + 1 - HALF_CHILD;
                lowest = new_node->low[0];
                return std::move(new_node);
            }
        }
    }

    bool merge_nodes(InternalNode* p, int x) {
        auto a = static_cast<InternalNode*>(p->child[x].get());
        auto b = static_cast<InternalNode*>(p->child[x + 1].get());
        b->low[0] = p->low[x + 1];
        int an = a->nchildren;
        int bn = b->nchildren;
        if (an + bn < MAX_CHILD) {
            for (int i = 0; i < bn; i++) {
                a->child[an + i] = std::move(b->child[i]);
                a->low[an + i] = b->low[i];
            }
            a->nchildren += bn;
            p->child[x + 1].reset();
            return true;
        } else {
            int n = (an + bn) / 2;
            if (an > n) {
                int move = an - n;
                for (int i = bn - 1; i >= 0; i--) {
                    b->child[i + move] = std::move(b->child[i]);
                    b->low[i + move] = b->low[i];
                }
                for (int i = 0; i < move; i++) {
                    b->child[i] = std::move(a->child[n + i]);
                    b->low[i] = a->low[n + i];
                }
            } else {
                int move = n - an;
                for (int i = 0; i < move; i++) {
                    a->child[an + i] = std::move(b->child[i]);
                    a->low[an + i] = b->low[i];
                }
                for (int i = 0; i < bn - move; i++) {
                    b->child[i] = std::move(b->child[i + move]);
                    b->low[i] = b->low[i + move];
                }
            }
            a->nchildren = n;
            b->nchildren = an + bn - n;
            p->low[x + 1] = b->low[0];
            return false;
        }
    }

    enum class DeleteResult { OK, REMOVED, NEED_REORG };

    DeleteResult delete_aux(Node* node, KEY key) {
        int joined = 0;
        if (node->isLeaf()) {
            auto leaf = static_cast<LeafNode*>(node);
            if (leaf->leaf_key == key) {
                return DeleteResult::REMOVED;
            } else {
                return DeleteResult::OK;
            }
        } else {
            auto internal = static_cast<InternalNode*>(node);
            int pos = locate_subtree(internal, key);
            auto condition = delete_aux(internal->child[pos].get(), key);
            if (condition == DeleteResult::OK)
                return DeleteResult::OK;
            if (condition == DeleteResult::NEED_REORG) {
                int sub = (pos == 0) ? 0 : pos - 1;
                bool joined = merge_nodes(internal, sub);
                if (joined) {
                    pos = sub + 1;
                }
            }
            if (condition == DeleteResult::REMOVED || joined) {
                for (int i = pos; i < internal->nchildren - 1; i++) {
                    internal->child[i] = std::move(internal->child[i + 1]);
                    internal->low[i] = internal->low[i + 1];
                }
                if (--internal->nchildren < HALF_CHILD) {
                    return DeleteResult::NEED_REORG;
                }
            }
            return DeleteResult::OK;
        }
    }

public:
    BTree() : root(nullptr) {}

    Node* search(KEY key) const {
        return root ? search_aux(root.get(), key) : nullptr;
    }

    void insert(KEY key) {
        if (!root) {
            root = std::make_unique<LeafNode>(key);
        } else {
            KEY lowest;
            auto new_node = insert_aux(root, key, lowest);
            if (new_node) {
                auto new_root = std::make_unique<InternalNode>();
                new_root->nchildren = 2;
                new_root->child[0] = std::move(root);
                new_root->child[1] = std::move(new_node);
                new_root->low[1] = lowest;
                root = std::move(new_root);
            }
        }
    }

    bool remove(KEY key) {
        if (!root)
            return false;
        auto result = delete_aux(root.get(), key);
        if (result == DeleteResult::REMOVED) {
            root.reset();
        } else if (result == DeleteResult::NEED_REORG && !root->isLeaf() && static_cast<InternalNode*>(root.get())->nchildren == 1) {
            auto internal_root = static_cast<InternalNode*>(root.get());
            root = std::move(internal_root->child[0]);
        }
        return result == DeleteResult::REMOVED;
    }

    void print() const {
        print_aux(root.get(), 0);
    }

private:
    void print_aux(Node* p, int depth) const {
        if (p->isLeaf()) {
            auto leaf = static_cast<LeafNode*>(p);
            std::cout << std::string(depth * 2, ' ') << "Leaf: " << leaf->leaf_key << std::endl;
        } else {
            auto internal = static_cast<InternalNode*>(p);
            std::cout << std::string(depth * 2, ' ') << "Internal: ";
            for (int i = 1; i < internal->nchildren; i++) {
                std::cout << internal->low[i] << " ";
            }
            std::cout << std::endl;
            for (int i = 0; i < internal->nchildren; i++) {
                print_aux(internal->child[i].get(), depth + 1);
            }
        }
    }
};

int main() {
    BTree tree;
    std::vector<int> data = {13, 5, 2, 7, 6, 21, 15};

    for (int key : data) {
        tree.insert(key);
    }

    std::cout << "Initial tree:" << std::endl;
    tree.print();

    if (tree.remove(5)) {
        std::cout << "\n5を削除しました" << std::endl;
    } else {
        std::cout << "\n5が見つかりません" << std::endl;
    }

    std::cout << "\nAfter deletion:" << std::endl;
    tree.print();

    return 0;
}