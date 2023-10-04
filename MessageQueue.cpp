#include "MessageQueue.h"

enum {
	defaultCapacity = 100,
	};


MessageQueue::MessageQueue(int capacity_in)
{
	capacity = (capacity_in > 0 ? capacity_in : defaultCapacity);
	ring = new Message[capacity];
	head = tail = 0;
}


MessageQueue::~MessageQueue()
{
	delete[] ring;
}


MessageQueue::Message* MessageQueue::back()
{
	if (is_full())
		return nullptr;

	return &ring[tail];
}

void MessageQueue::push()
{
	tail = (tail + 1) % capacity;
}

void MessageQueue::send(int message)
{
	Message* msg = back();
	if (msg) {
		msg->id = message;
		push();
		}
}

void MessageQueue::send(int message, void* param)
{
	Message* msg = back();
	if (msg) {
		msg->id = message;
		msg->param = param;
		push();
		}
}

void MessageQueue::send(int message, int64_t num)
{
	Message* msg = back();
	if (msg) {
		msg->id = message;
		msg->num = num;
		push();
		}
}


MessageQueue::Message MessageQueue::pop_front()
{
	if (is_empty())
		return { -1 };
	Message result = ring[head];
	pop();
	return result;
}

MessageQueue::Message* MessageQueue::front()
{
	if (is_empty())
		return nullptr;

	return &ring[head];
}

void MessageQueue::pop()
{
	head = (head + 1) % capacity;
}


bool MessageQueue::is_empty()
{
	return tail == head;
}


bool MessageQueue::is_full()
{
	return (tail + 1) % capacity == head;
}



