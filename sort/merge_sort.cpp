#include <iostream>
#include <memory>

template<typename T>
class LinkedList {
private:
    struct Node {
        T value;
        std::shared_ptr<Node> next;
        Node(T val) : value(val), next(nullptr) {}
    };

    std::shared_ptr<Node> head;

public:
    void push_back(T value) {
        auto new_node = std::make_shared<Node>(value);
        if (!head) {
            head = new_node;
            return;
        }
        auto current = head;
        while (current->next) {
            current = current->next;
        }
        current->next = new_node;
    }

    void print() const {
        auto current = head;
        while (current) {
            std::cout << current->value << " ";
            current = current->next;
        }
        std::cout << std::endl;
    }

    void merge_sort() {
        head = merge_sort_recursive(head);
    }

private:
    std::shared_ptr<Node> merge_sort_recursive(std::shared_ptr<Node> node) {
        if (!node || !node->next) {
            return node;
        }

        auto middle = get_middle(node);
        auto right_half = middle->next;
        middle->next = nullptr;

        auto left_sorted = merge_sort_recursive(node);
        auto right_sorted = merge_sort_recursive(right_half);

        return merge(left_sorted, right_sorted);
    }

    std::shared_ptr<Node> get_middle(std::shared_ptr<Node> head) {
        auto slow = head;
        auto fast = head->next;

        while (fast && fast->next) {
            slow = slow->next;
            fast = fast->next->next;
        }

        return slow;
    }

    std::shared_ptr<Node> merge(std::shared_ptr<Node> left, std::shared_ptr<Node> right) {
        if (!left) return right;
        if (!right) return left;

        std::shared_ptr<Node> result;
        if (left->value <= right->value) {
            result = left;
            result->next = merge(left->next, right);
        } else {
            result = right;
            result->next = merge(left, right->next);
        }

        return result;
    }
};

int main() {
    LinkedList<int> list;

    list.push_back(1);
    list.push_back(5);
    list.push_back(4);
    list.push_back(9);
    list.push_back(2);

    std::cout << "Original list: ";
    list.print();

    list.merge_sort();

    std::cout << "Sorted list: ";
    list.print();

    return 0;
}
