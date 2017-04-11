#include <algorithm>
#include <iterator>

enum class NodeColor{BLACK, RED};

template<class ValueType>
class Set;

template<class ValueType>
class SetIterator;

template<class ValueType>
class Node {
 private:
    friend class Set<ValueType>;
    friend class SetIterator<ValueType>;

    using PNode = Node *;

    const ValueType data;
    NodeColor color;
    PNode left;
    PNode right;
    PNode parent;

    Node(const ValueType& data, NodeColor color, PNode left, PNode right, PNode parent);
    Node(const Node& other);

    PNode minimum(PNode terminal);
    PNode maximum(PNode terminal);

    PNode next(PNode terminal);
    PNode prev(PNode terminal);
};


template<class ValueType>
class Set {
 public:
    using iterator = SetIterator<ValueType>;

    Set();

    template<typename It>
    Set(It first, It last);

    Set(std::initializer_list<ValueType> init);

    Set(const Set& other);

    Set(Set&& other);

    ~Set();

    Set& operator = (const Set& other);
    Set& operator = (Set&& other);

    iterator begin() const;
    iterator end() const;

    bool empty() const;
    size_t size() const;

    void insert(const ValueType& value);
    void erase(iterator old_node);
    void erase(const ValueType& value);

    void swap(Set& other);

    iterator find(const ValueType& value) const;
    iterator lower_bound(const ValueType& value) const;

 private:
    friend class SetIterator<ValueType>;

    using PNode = Node<ValueType> *;

    PNode terminal_;
    PNode root_;
    size_t size_;

    PNode head_;
    PNode tail_;

    void transplant(PNode x, PNode y);

    void left_rotate(PNode x);
    void right_rotate(PNode x);

    void insert_fix_up(PNode v);
    void erase_fix_up(PNode v);
};


template<class ValueType>
class SetIterator: public std::iterator<std::input_iterator_tag, ValueType> {
 public:
    SetIterator() = default;
    SetIterator(const SetIterator& other);
    SetIterator(SetIterator&& other);

    SetIterator& operator = (const SetIterator& other);
    SetIterator& operator = (SetIterator&& other);

    SetIterator& operator++ ();
    SetIterator operator++ (int);

    SetIterator& operator-- ();
    SetIterator operator-- (int);

    const ValueType& operator * () const;
    const ValueType * operator -> () const;

    bool operator == (const SetIterator& other) const;
    bool operator != (const SetIterator& other) const;

 private:
    friend class Set<ValueType>;

    using PNode = Node<ValueType> *;

    SetIterator(const Set<ValueType> * container, PNode current);

    const Set<ValueType> * container;
    PNode current;
};


template<class ValueType>
Node<ValueType>::Node(
    const ValueType& data,
    NodeColor color,
    PNode left,
    PNode right,
    PNode parent
)   : data(data)
    , color(color)
    , left(left)
    , right(right)
    , parent(parent) {
}

template<class ValueType>
Node<ValueType>::Node(const Node<ValueType>& other)
    : data(other.data)
    , color(other.color)
    , left(other.left)
    , right(other.right)
    , parent(other.parent) {
}

template<class ValueType>
Node<ValueType> * Node<ValueType>::minimum(PNode terminal) {
    PNode x = this;
    while (x->left != terminal) {
        x = x->left;
    }
    return x;
}

template<class ValueType>
Node<ValueType> * Node<ValueType>::maximum(PNode terminal) {
    PNode x = this;
    while (x->right != terminal) {
        x = x->right;
    }
    return x;
}

template<class ValueType>
Node<ValueType> * Node<ValueType>::next(PNode terminal) {
    PNode x;
    if (this->right != terminal) {
        x = this->right->minimum(terminal);
    } else {
        x = this->parent;
        PNode y = this;
        while (x != terminal && y == x->right) {
            y = x;
            x = x->parent;
        }
    }
    return x;
}

template<class ValueType>
Node<ValueType> * Node<ValueType>::prev(PNode terminal) {
    PNode x;
    if (this->left != terminal) {
        x = this->left->maximum(terminal);
    } else {
        x = this->parent;
        PNode y = this;
        while (x != terminal && y == x->left) {
            y = x;
            x = x->parent;
        }
    }
    return x;
}


template<class ValueType>
Set<ValueType>::Set()
    : terminal_(new Node<ValueType>(ValueType(), NodeColor::BLACK, nullptr, nullptr, nullptr))
    , size_(0) {
    terminal_->left = terminal_;
    terminal_->right = terminal_;
    terminal_->parent = terminal_;
    root_ = terminal_;
    head_ = terminal_;
    tail_ = terminal_;
}

template<class ValueType>
template<typename It>
Set<ValueType>::Set(It first, It last) : Set() {
    while (first != last) {
        insert(*first);
        ++first;
    }
}

template<class ValueType>
Set<ValueType>::Set(std::initializer_list<ValueType> init)
    : Set<ValueType>(init.begin(), init.end()) {
}

template<class ValueType>
Set<ValueType>::Set(const Set<ValueType>& other)
    : Set<ValueType>(other.begin(), other.end()) {
}

template<class ValueType>
Set<ValueType>::Set(Set<ValueType>&& other)
    : terminal_(std::move(other.terminal_))
    , root_(std::move(other.root_))
    , size_(std::move(other.size_))
    , head_(std::move(other.head_))
    , tail_(std::move(other.tail_)) {
}

template<class ValueType>
Set<ValueType>::~Set() {
    while (!empty()) {
        erase(begin());
    }
    delete(terminal_);
}

template<class ValueType>
Set<ValueType>& Set<ValueType>::operator = (const Set<ValueType>& other) {
    if (this == &other) {
        return *this;
    }
    return *this = Set<ValueType>(other.begin(), other.end());
}

template<class ValueType>
Set<ValueType>& Set<ValueType>::operator = (Set<ValueType>&& other) {
    swap(other);
    return *this;
}


template<class ValueType>
typename Set<ValueType>::iterator Set<ValueType>::begin() const {
    return {this, head_};
}

template<class ValueType>
typename Set<ValueType>::iterator Set<ValueType>::end() const {
    return {this, terminal_};
}


template<class ValueType>
bool Set<ValueType>::empty() const {
    return size() == 0;
}

template<class ValueType>
size_t Set<ValueType>::size() const {
    return size_;
}


template<class ValueType>
void Set<ValueType>::insert(const ValueType& value) {
    PNode y = terminal_;
    PNode x = root_;
    while (x != terminal_) {
        y = x;
        if (value < x->data) {
            x = x->left;
        } else if (x->data < value) {
            x = x->right;
        } else {
            return;
        }
    }

    PNode z = new Node<ValueType>(value, NodeColor::RED, terminal_, terminal_, y);

    if (head_ == terminal_ || value < head_->data) {
        head_ = z;
    }
    if (tail_ == terminal_ || tail_->data < value) {
        tail_ = z;
    }

    if (y == terminal_) {
        root_ = z;
    } else if (value < y->data) {
        y->left = z;
    } else {
        y->right = z;
    }

    ++size_;
    insert_fix_up(z);
}


template<typename ValueType>
void Set<ValueType>::erase(iterator old_node) {
    if (old_node == end()) {
        return;
    }

    PNode z = old_node.current;

    if (head_ == z) {
        head_ = head_->next(terminal_);
    }
    if (tail_ == z) {
        tail_ = tail_->prev(terminal_);
    }

    PNode x, y;
    y = z;

    NodeColor original_color = y->color;
    if (z->left == terminal_) {
        x = z->right;
        transplant(z, z->right);
    } else if (z->right == terminal_) {
        x = z->left;
        transplant(z, z->left);
    } else {
        y = z->right->minimum(terminal_);
        original_color = y->color;
        x = y->right;
        if (y->parent == z) {
            x->parent = y;
        } else {
            transplant(y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        transplant(z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    if (original_color == NodeColor::BLACK) {
        erase_fix_up(x);
    }

    --size_;
    delete(z);
}

template<class ValueType>
void Set<ValueType>::erase(const ValueType& value) {
    erase(find(value));
}


template<class ValueType>
void Set<ValueType>::swap(Set<ValueType>& other) {
    std::swap(terminal_, other.terminal_);
    std::swap(root_, other.root_);
    std::swap(size_, other.size_);
    std::swap(head_, other.head_);
    std::swap(tail_, other.tail_);
}

template<class ValueType>
void swap(Set<ValueType>& left, Set<ValueType>& right) {
    left.swap(right);
}


template<class ValueType>
typename Set<ValueType>::iterator Set<ValueType>::find(const ValueType& value) const {
    PNode current = root_;
    while (current != terminal_) {
        if (value < current->data) {
            current = current->left;
        } else if (current->data < value) {
            current = current->right;
        } else {
            break;
        }
    }
    return {this, current};
}

template<class ValueType>
typename Set<ValueType>::iterator Set<ValueType>::lower_bound(const ValueType& value) const {
    PNode res = terminal_;
    PNode current = root_;
    while (current != terminal_) {
        if (current->data < value) {
            current = current->right;
        } else if (value < current->data) {
            if (res == terminal_ || current->data < res->data) {
                res = current;
            }
            current = current->left;
        } else {
            res = current;
            break;
        }
    }
    return {this, res};
}


template<class ValueType>
void Set<ValueType>::transplant(PNode x, PNode y) {
    if (x->parent == terminal_) {
        root_ = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }
    y->parent = x->parent;
}

template<class ValueType>
void Set<ValueType>::left_rotate(PNode x) {
    PNode y = x->right;

    x->right = y->left;
    if (y->left != terminal_) {
        y->left->parent = x;
    }

    if (y != terminal_) {
        y->parent = x->parent;
    }
    if (x->parent == terminal_) {
        root_ = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }

    y->left = x;
    if (x != terminal_) {
        x->parent = y;
    }
}

template<class ValueType>
void Set<ValueType>::right_rotate(PNode x) {
    PNode y = x->left;

    x->left = y->right;
    if (y->right != terminal_) {
        y->right->parent = x;
    }

    if (y != terminal_) {
        y->parent = x->parent;
    }
    if (x->parent == terminal_) {
        root_ = y;
    } else if (x == x->parent->right) {
        x->parent->right = y;
    } else {
        x->parent->left = y;
    }

    y->right = x;
    if (x != terminal_) {
        x->parent = y;
    }
}


template<class ValueType>
void Set<ValueType>::insert_fix_up(PNode x) {
    while (x->parent->color == NodeColor::RED) {
        if (x->parent == x->parent->parent->left) {
            PNode y = x->parent->parent->right;
            if (y->color == NodeColor::RED) {
                x->parent->color = NodeColor::BLACK;
                y->color = NodeColor::BLACK;
                x->parent->parent->color = NodeColor::RED;
                x = x->parent->parent;
            } else {
                if (x == x->parent->right) {
                    x = x->parent;
                    left_rotate(x);
                }
                x->parent->color = NodeColor::BLACK;
                x->parent->parent->color = NodeColor::RED;
                right_rotate(x->parent->parent);
            }
        } else {
            PNode y = x->parent->parent->left;
            if (y->color == NodeColor::RED) {
                x->parent->color = NodeColor::BLACK;
                y->color = NodeColor::BLACK;
                x->parent->parent->color = NodeColor::RED;
                x = x->parent->parent;
            } else {
                if (x == x->parent->left) {
                    x = x->parent;
                    right_rotate(x);
                }
                x->parent->color = NodeColor::BLACK;
                x->parent->parent->color = NodeColor::RED;
                left_rotate(x->parent->parent);
            }
        }
    }
    root_->color = NodeColor::BLACK;
}

template<class ValueType>
void Set<ValueType>::erase_fix_up(PNode x) {
    while (x != root_ && x->color == NodeColor::BLACK) {
        if (x == x->parent->left) {
            PNode w = x->parent->right;
            if (w->color == NodeColor::RED) {
                w->color = NodeColor::BLACK;
                x->parent->color = NodeColor::RED;
                left_rotate(x->parent);
                w = x->parent->right;
            }
            if (w->left->color == NodeColor::BLACK && w->right->color == NodeColor::BLACK) {
                w->color = NodeColor::RED;
                x = x->parent;
            } else {
                if (w->right->color == NodeColor::BLACK) {
                    w->left->color = NodeColor::BLACK;
                    w->color = NodeColor::RED;
                    right_rotate(w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = NodeColor::BLACK;
                w->right->color = NodeColor::BLACK;
                left_rotate(x->parent);
                x = root_;
            }
        } else {
            PNode w = x->parent->left;
            if (w->color == NodeColor::RED) {
                w->color = NodeColor::BLACK;
                x->parent->color = NodeColor::RED;
                right_rotate(x->parent);
                w = x->parent->left;
            }
            if (w->right->color == NodeColor::BLACK && w->left->color == NodeColor::BLACK) {
                w->color = NodeColor::RED;
                x = x->parent;
            } else {
                if (w->left->color == NodeColor::BLACK) {
                    w->right->color = NodeColor::BLACK;
                    w->color = NodeColor::RED;
                    left_rotate(w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = NodeColor::BLACK;
                w->left->color = NodeColor::BLACK;
                right_rotate(x->parent);
                x = root_;
            }
        }
    }
    x->color = NodeColor::BLACK;
}


template<class ValueType>
SetIterator<ValueType>::SetIterator(
    const Set<ValueType> * container,
    PNode current
)   : container(container)
    , current(current) {
}

template<class ValueType>
SetIterator<ValueType>::SetIterator(const SetIterator& other)
    : container(other.container)
    , current(other.current) {
}

template<class ValueType>
SetIterator<ValueType>::SetIterator(SetIterator&& other)
    : container(std::move(other.container))
    , current(std::move(other.current)) {
}

template<class ValueType>
SetIterator<ValueType>& SetIterator<ValueType>::operator = (const SetIterator& other) {
    if (this == &other) {
        return *this;
    }
    return *this = SetIterator<ValueType>(other);
}

template<class ValueType>
SetIterator<ValueType>& SetIterator<ValueType>::operator = (SetIterator&& other) {
    std::swap(container, other.container);
    std::swap(current, other.current);
    return *this;
}


template<class ValueType>
SetIterator<ValueType>& SetIterator<ValueType>::operator++ () {
    current = current->next(container->terminal_);
    return *this;
}

template<class ValueType>
SetIterator<ValueType> SetIterator<ValueType>::operator++ (int) {
    SetIterator tmp(*this);
    ++(*this);
    return tmp;
}

template<class ValueType>
SetIterator<ValueType>& SetIterator<ValueType>::operator-- () {
    if (current == container->terminal_) {
        current = container->tail_;
    } else {
        current = current->prev(container->terminal_);
    }
    return *this;
}

template<class ValueType>
SetIterator<ValueType> SetIterator<ValueType>::operator-- (int) {
    SetIterator tmp(*this);
    --(*this);
    return tmp;
}


template<class ValueType>
const ValueType& SetIterator<ValueType>::operator * () const {
    return current->data;
}

template<class ValueType>
const ValueType * SetIterator<ValueType>::operator -> () const {
    return &current->data;
}


template<class ValueType>
bool SetIterator<ValueType>::operator == (const SetIterator& other) const {
    return current == other.current;
}

template<class ValueType>
bool SetIterator<ValueType>::operator != (const SetIterator& other) const {
    return !(*this == other);
}
