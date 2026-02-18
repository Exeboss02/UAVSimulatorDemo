#pragma once

template <typename T>
class MinPriorityQueue {
public:
	MinPriorityQueue() : head(nullptr) {}
	MinPriorityQueue(MinPriorityQueue& other) = delete;
	MinPriorityQueue& operator=(const MinPriorityQueue&) = delete;
	~MinPriorityQueue();

	void Enqueue(const T data, const unsigned int priority);
	T Peek() const;
	T Dequeue();
	size_t Size() const;
	bool IsEmpty() const;
	void Clear();

private:
	struct Node {
		unsigned int priority;
		T data;
		Node* next;
		Node(const T data, const unsigned int priority) : data(data), priority(priority), next(nullptr) {}
	};
	Node* head;
	void ReplaceNode(Node* newNode, Node* prevNode);
};


template <typename T>
inline MinPriorityQueue<T>::~MinPriorityQueue() {
	while (this->head) {
		Node* temp = this->head;
		this->head = this->head->next;
		delete temp;
	}
}

/// <summary>
/// Adds an element to the priority queue with the specified priority.
/// </summary>
/// <param name="data">The data element to add to the queue.</param>
/// <param name="priority">The priority value for the element. Lower values indicate higher priority.</param>
template <typename T>
inline void MinPriorityQueue<T>::Enqueue(const T data, const unsigned int priority) {
	Node* newNode = new Node(data, priority);
	if (this->head == nullptr) {
		this->head = newNode;
		return;
	}

	Node* walker = this->head;
	while (walker->next && walker->next->priority < priority) {
		/*if (walker == data) {
			this->ReplaceNode(newNode, walker);
			return;
		}*/
		walker = walker->next;
	}

	if (walker->next == nullptr) {
		walker->next = newNode;
	} 
	else {
		newNode->next = walker->next;
		walker->next = newNode;
	}
}

template <typename T>
inline T MinPriorityQueue<T>::Peek() const {
	if (this->IsEmpty()) {
		throw std::runtime_error("PriorityQueue is empty.");
	}
	return this->head->data;
}

/// <summary>
/// Removes and returns the lowest priority element from the queue.
/// </summary>
template <typename T>
inline T MinPriorityQueue<T>::Dequeue() {
	if (this->IsEmpty()) {
		throw std::runtime_error("MinPriorityQueue is empty.");
	}
	
	Node* current = this->head;
	T data = current->data;
	this->head = this->head->next;
	delete current;

	return data;
}

/// <summary>
/// Returns number of elements in the queue.
/// </summary>
template <typename T>
inline size_t MinPriorityQueue<T>::Size() const {
	size_t count = 0;
	Node* walker = this->head;
	while (walker) {
		count++;
		walker = walker->next;
	}
	return count;
}

/// <summary>
/// Is the min priority queue empty?
/// </summary>
template <typename T>
inline bool MinPriorityQueue<T>::IsEmpty() const {
	return this->head == nullptr;
}

/// <summary>
/// Clear the min priority queue by deleting all nodes and resetting the head pointer to nullptr.
/// </summary>
template <typename T>
inline void MinPriorityQueue<T>::Clear() {
	while (this->head) {
		Node* temp = this->head;
		this->head = this->head->next;
		delete temp;
	}
}

template <typename T>
inline void MinPriorityQueue<T>::ReplaceNode(Node* newNode, Node* prevNode) {
	if (prevNode == this->head) {
		newNode->next = prevNode->next;
		delete prevNode;
		this->head = newNode;
	}
	else {
		Node* walker = this->head;
		while (walker->next != prevNode) {
			walker = walker->next;
		}
		newNode->next = prevNode->next;
		delete prevNode;
		walker->next = newNode;
	}
}
