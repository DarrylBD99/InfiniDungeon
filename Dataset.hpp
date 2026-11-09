#pragma once
#include <type_traits>

namespace Dataset {
	template<typename T>
	class Array {
	private:
		T* data;
		unsigned int maxSize;
		unsigned int size = 0;
	public:
		Array(unsigned int maxSize_ = 1024) : maxSize(maxSize_) {
			data = new T[maxSize]();
		}

		~Array() {
			delete[] data;
		}

		T& Get(unsigned int index) const {
			return data[index];
		}

		unsigned int AddToClosestEmptyIndex(const T& value) {
			for (unsigned int i = 0; i < maxSize; i++) {
				if (data[i] != T{}) continue;
				data[i] = value;
				if (i >= size) size = i + 1;
				return (int)i;
			}
			return maxSize;
		}

		void SetValue(const T& value, const unsigned int index) {
			if (index >= maxSize) return;
			if (index >= size) size = index + 1;

			data[index] = value;
		}

		void RemoveAtIndex(unsigned int index) {
			if (index >= size)
				return;

			// Check if is storing pointer
			if constexpr (std::is_pointer_v<T>) {
				delete data[index];
				data[index] = nullptr;
			}
			else data[index] = T{};
		}

		bool HasData(const T& value) const {
			for (unsigned int i = 0; i < size; i++)
				if (data[i] == value) return true;
			return false;
		}

		void ResetSize() {
			while (size > 0 && Get(size - 1) == T{})
				size--;
		}

		unsigned int GetSize() const { return size; }
		unsigned int GetMaxSize() const { return maxSize; }

		// Move Constructor
		Array(Array<T>&& array_) noexcept : data(array_.data), size(array_.size), maxSize(array_.maxSize) {
			array_.size = 0;
			array_.maxSize = 0;
			array_.data = nullptr;
		}

		// Move Operator
		Array<T>& operator=(Array<T>&& array_) noexcept {
			if (this == &array_) return *this;

			delete[] data;
			size = array_.size;
			maxSize = array_.maxSize;
			data = array_.data;

			array_.size = 0;
			array_.maxSize = 0;
			array_.data = nullptr;

			return *this;
		}

		// Copy Constructor
		Array(const Array<T>& array_) : data(new T[array_.maxSize]()), size(array_.size), maxSize(array_.maxSize) {
			for (unsigned int i = 0; i < size; i++)
				data[i] = array_.data[i];
		}

		// Copy Operator
		Array<T>& operator=(const Array<T>& array_) {
			if (this == &array_) return *this;

			delete[] data;
			size = array_.size;
			maxSize = array_.maxSize;
			data = new T[maxSize]();
			for (unsigned int i = 0; i < size; i++)
				data[i] = array_.data[i];

			return *this;
		}

		T* begin() { return data; }
		T* end() { return data + size; }
	};

	template <typename T>
	struct Node {
		Node* next;
		T data;

		Node(const T& data_) : data(data_), next(nullptr) {}
	};

	template <typename T>
	class Queue {
	private:
		Node<T>* head;
		Node<T>* tail;
	public:
		Queue() {
			head = tail = nullptr;
		}

		~Queue() {
			while (Dequeue());
		}

		Node<T>* GetHead() {
			return head;
		}

		Node<T>* GetTail() {
			return tail;
		}

		void Enqueue(const T& data) {
			Node<T>* node = new Node<T>(data);
			if (tail == nullptr) {
				tail = head = node;
			}
			else {
				tail->next = node;
				tail = node;
			}
		}

		bool Dequeue() {
			if (head == nullptr) return false;
			Node<T>* n = head;
			if (head != tail) head = head->next;
			else head = tail = nullptr;

			delete n;

			return true;
		}

		bool Dequeue(T& data) {
			if (head == nullptr) return false;
			Node<T>* n = head;
			if (head != tail) head = head->next;
			else head = tail = nullptr;

			data = n->data;
			delete n;

			return true;
		}

		// Move Constructor
		Queue(Queue<T>&& queue_) noexcept : head(queue_.head), tail(queue_.tail) {
			queue_.head = nullptr;
			queue_.tail = nullptr;
		}

		// Move Operator
		Queue<T>& operator=(Queue<T>&& queue_) noexcept {
			if (this == &queue_) return *this;
			while (Dequeue());

			head = queue_.head;
			tail = queue_.tail;
			queue_.head = nullptr;
			queue_.tail = nullptr;

			return *this;
		}

		// Copy Constructor
		Queue(const Queue<T>& queue_) {
			head = tail = nullptr;

			Node<T>* node = queue_.head;

			while (node != nullptr) {
				Enqueue(node->data);
				node = node->next;
			}
		}

		// Copy Operator
		Queue<T>& operator=(const Queue<T>& queue_) {
			if (this == &queue_) return *this;
			while (Dequeue());
			head = tail = nullptr;

			Node<T>* node = queue_.head;

			while (node != nullptr) {
				Enqueue(node->data);
				node = node->next;
			}

			return *this;
		}
	};
};