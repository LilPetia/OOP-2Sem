#include <vector>
#include <type_traits>
#include <set>
#include <memory>
#include <iostream>

#define bucket size_t
#define max_chunk 24

template<size_t chunkSize>
class FixedAllocator {
public:
//  static size_t bucketSize;
    static bucket *buckets;
    static std::vector<bucket *> freed;
    static size_t added;
//  static int begin;

    bucket *add() {

        if (freed.empty()) {
            buckets += added;
            return buckets;

        } else {
            bucket *returned = freed.back();
            freed.pop_back();
            return returned;
        }
    }

    void del(bucket *x) {
        freed.push_back(x);

    }


};

template<size_t chunkSize>
bucket *FixedAllocator<chunkSize>::buckets = reinterpret_cast<bucket *>(malloc(sizeof(bucket) * 8000000));
template<size_t chunkSize>
std::vector<bucket *> FixedAllocator<chunkSize>::freed = std::vector<bucket *>();
template<size_t chunkSize>
size_t FixedAllocator<chunkSize>::added = chunkSize / sizeof(bucket);

template<typename type>
class FastAllocator {
public:


    FastAllocator() = default;

    template<typename T>
    FastAllocator(const FastAllocator<T> &) {}

    template<typename T>
    FastAllocator &operator=(const FastAllocator<T> &) {
        return *this;
    }

    template<typename T>
    struct rebind {
        using other = FastAllocator<T>;
    };
    using value_type = type;

    type *allocate(size_t sz) {
        if (sz != 1 || sizeof(type) > max_chunk) {
            return reinterpret_cast<type *>(malloc(sizeof(type) * sz));

        } else {
            return reinterpret_cast<type *>(FixedAllocator<sizeof(type)>().add());
        }
    }

    void deallocate(type *x, size_t sz) {
        if (sz != 1 || sizeof(type) > max_chunk) {
            free(x);
        } else {
            FixedAllocator<sizeof(type)>().del(reinterpret_cast<bucket *>(x));
        }
    }

    ~FastAllocator() = default;
};

template<typename T, typename Allocator = std::allocator<T>>

class List {
private:
    struct Node {
        T data;
        Node *next = nullptr;
        Node *previous = nullptr;
    };
    Node *root;
    size_t sz = 0;

    void fill_Node(Node node, T data, Node *next, Node *previous) {
        node.data = data;
        node.next = next;
        node.previous = previous;
    }

    typename std::allocator_traits<Allocator>::template rebind_alloc<Node> alloc;

    Node *make_root() {
        root = alloc.allocate(1);
        root->next = root;
        root->previous = root;
        return root;
    }

    void del_root() {
        alloc.deallocate(root, 1);
    }

    Node *insert_node_after(Node *node_bound, const T &value) {
        Node *node = alloc.allocate(1);
        node->data = value;
        node->previous = node_bound;
        node->next = node_bound->next;
        node_bound->next->previous = node;
        ++sz;
        node_bound->next = node;
        return node;
    }

    Node *insert_node_before_emphty(Node *node_bound) {
        Node *node = alloc.allocate(1);
        ++sz;
        //std::allocator_traits<typename std::allocator_traits<Allocator>::template rebind_alloc<Node>>::construct(alloc, node, node, node_bound->next);
        //node->data = reinterpret_cast<T>(node->data);
        new(&(node->data)) T();
        node->next = node_bound;
        node->previous = node_bound->previous;
        node_bound->previous->next = node;
        node_bound->previous = node;
        return node;
    }


    Node *insert_node_before(Node *node_bound, const T &value) {
        Node *node = alloc.allocate(1);
        ++sz;
        new(&(node->data)) T(value);//////////////
        node->next = node_bound;
        node->previous = node_bound->previous;
        node_bound->previous->next = node;
        node_bound->previous = node;
        return node;
    }

    void delete_node(Node *node) {
        sz--;
        node->previous->next = node->next;
        node->next->previous = node->previous;
        node->data.~T();
        alloc.deallocate(node, 1);
    }

    void delete_root() {
        alloc.deallocate(root, 1);
    }

public:

    explicit List(const Allocator &allocator = Allocator()) : alloc(allocator) {
        make_root();
    }

    List(size_t count, const T &value, const Allocator &alloc = Allocator()) : alloc(alloc) {
        make_root();
        for (int i = 0; i < count; ++i) {
            insert_node_before(root, value);
        }
    }

    size_t size() const noexcept {
        return sz;
    }

    void destroy() {

        while (root->previous != root) delete_node(root->previous);

    }

    List(const List<T, Allocator> &list) {
        alloc = std::allocator_traits<typename std::allocator_traits<Allocator>::template rebind_alloc<Node>>::select_on_container_copy_construction(
                list.get_allocator());
        //destroy();
        //if (root != nullptr) del_root();
        make_root();
        for (auto i = list.begin(); i != list.end(); ++i)
            insert_node_before(root, *i);
    }

    List &operator=(const List<T, Allocator> &list) {
        if (std::allocator_traits<typename std::allocator_traits<Allocator>::template rebind_alloc<Node>>::propagate_on_container_copy_assignment::value) {
            alloc = list.get_allocator();
        }
        destroy();
        if (root != nullptr)del_root();
        make_root();
        for (auto i = list.begin(); i != list.end(); i++)
            insert_node_before(root, i.ptr->data);
        return *this;
    }

    List(size_t count, const Allocator &alloc = Allocator()) : alloc(alloc) {
        make_root();
        for (size_t i = 0; i < count; ++i) {
            insert_node_before_emphty(root);
        }
    }

    ~List() {
        while (sz > 0) {
            pop_back();
        }
        delete_root();
    }

    auto get_allocator() const {
        return alloc;
    }

    void push_back(const T &data) {
        insert_node_before(root, data);
    }

    void push_front(const T &data) {
        insert_node_after(root, data);
    }

    void pop_front() {
        if (root->next == root) return;
        delete_node(root->next);
    }

    void pop_back() {
        if (root->previous == root) return;
        delete_node(root->previous);
    }

    template<bool Const>
    struct c_iterator {
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;////////////
        using value_type = std::conditional_t<Const, const T, T>;;
        using pointer = value_type *;
        using reference = value_type &;
        Node *ptr;

        c_iterator(Node *node) : ptr(node) {}


        template<bool Another>
        c_iterator(c_iterator<Another> &it) {
            this->ptr = it.ptr;
        }

        ~c_iterator() {
            //~ptr;/////////////////////////////////////////////////
        }

        c_iterator &operator++() {
            ptr = ptr->next;
            return *this;
        }

        c_iterator operator++(int) {
            c_iterator<Const> it(*this);
            ptr = ptr->next;
            return it;//////////////
        }

        c_iterator &operator--() {
            ptr = ptr->previous;
            return *this;
        }

        c_iterator operator--(int) {
            c_iterator<Const> it = this;
            return it++;
        }

        pointer operator->() {
            return &(ptr->data);////
        }/*
        reference operator&(){
            return ptr->data;
        }*/
        reference operator*() {
            return (ptr->data);////////////////////////////////////////
        }

        template<bool Another>
        bool operator==(c_iterator<Another> another) {////?????doesnot work with &
            return another.ptr == ptr;
        }

        template<bool Another>
        bool operator!=(c_iterator<Another> another) {
            return another.ptr != ptr;
        }

        operator c_iterator<true>() const {
            return c_iterator<true>(
                    this->ptr);///////??????????????????????????????????????????????????????????????????????????
        }

    };

    using iterator = c_iterator<false>;
    using const_iterator = c_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    iterator begin() {
        return iterator(root->next);
    }

    const_iterator begin() const {
        return const_iterator(root->next);
    }

    const_iterator cbegin() const {
        return const_iterator(root->next);
    }

    iterator end() {
        return iterator(root);
    }

    const_iterator end() const {
        return const_iterator(root);
    }

    const_iterator cend() const {
        return const_iterator(root);
    }

    reverse_iterator rbegin() {
        return reverse_iterator(root);
    }

    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(root);
    }

    const_reverse_iterator crbegin() const {
        return const_reverse_iterator(root);
    };

    reverse_iterator rend() {
        return reverse_iterator(root->next);
    }

    const_reverse_iterator rend() const {
        return creverse_iterator(root->next);
    }

    const_reverse_iterator crend() const {
        return creverse_iterator(root->next);
    }

    template<bool Const>
    void insert(const c_iterator<Const> &it, const T &data) {
        insert_node_before(it.ptr, data);
    }

    template<bool Const>
    void erase(const c_iterator<Const> &it) {
        delete_node(it.ptr);
    }
};
