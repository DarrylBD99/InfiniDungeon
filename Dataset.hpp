#pragma once

namespace Dataset {
	template<typename T>
	
	class Array {
	private:
		T* data;
	public:
		const unsigned int maxSize;

		Array(unsigned int maxSize_ = 1024) : maxSize(maxSize_) {
			data = new T[maxSize]();
		}

		~Array() {
			delete[] data;
		}

		T& Get(unsigned int index) const {
			return data[index];
		}

		void SetValue(const T& value, unsigned int index) {
			data[index] = value;
		}

		void RemoveAtIndex(unsigned int index) {
			delete data[index];
			data[index] = nullptr;
		}

		T* begin() { return data; }
		T* end() { return data + maxSize; }
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