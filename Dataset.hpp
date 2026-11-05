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
				if (data[i] == T{}) {
					data[i] = value;

					if (i >= size)
						size = i + 1;

					return (int)i;
				}
			}
			return maxSize;
		}

		void SetValue(const T& value, unsigned int index) {
			if (index >= maxSize)
				return;

			if (index >= size)
				size = index + 1;

			data[index] = value;
		}

		void RemoveAtIndex(unsigned int index) {
			if (index >= size)
				return;

			if constexpr (std::is_pointer_v<T>) {
				delete data[index];
				data[index] = nullptr;
			}
			else {
				data[index] = T{};
			}
		}

		bool HasData(const T& value) const {
			for (unsigned int i = 0; i < size; i++) {
				if (data[i] == value) {
					return true;
				}
			}

			return false;
		}

		void ResetSize() {
			while (size > 0 && Get(size - 1) == T{})
				size--;
		}

		unsigned int GetSize() const { return size; }

		unsigned int GetMaxSize() const { return maxSize; }

		Array(Array<T>&& arrayNew) noexcept : data(arrayNew.data), size(arrayNew.size), maxSize(arrayNew.maxSize) {
			arrayNew.size = 0;
			arrayNew.maxSize = 0;
			arrayNew.data = nullptr;
		}

		Array<T>& operator=(Array<T>&& arrayNew) noexcept {
			if (this != &arrayNew) {
				delete[] data;

				size = arrayNew.size;
				maxSize = arrayNew.maxSize;
				data = arrayNew.data;

				arrayNew.size = 0;
				arrayNew.maxSize = 0;
				arrayNew.data = nullptr;
			}

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

		Node<T>* GetHead() {
			return head;
		}

		Node<T>* GetTail() {
			return tail;
		}

		void Enqueue(const T& data) {
			Node<T>* node = new Node<T>;

			if (tail == nullptr) {
				tail = head = node;
			} else {
				tail->next = node;
				tail = node;
			}
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

	};
};