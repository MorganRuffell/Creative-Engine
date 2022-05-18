#pragma once


/// <summary>
/// This is a CLinkedListNode. This can contain other types of classes that we can add elements into.
/// </summary>
/// <typeparam name="T"></typeparam>
template<typename T>
class CLinkedListNode
{
public:

	CLinkedListNode* Next;
	CLinkedListNode* Previous;

};


/// <summary>
/// This is the basic implementation of a linkedlist, 
/// creative uses a specialized version that is designed for it's own use cases
/// </summary>
template<typename T>
class CLinkedList
{
	/// <summary>
	/// Generic constructor -- This allows us to start a linkedlist of any class.
	/// </summary>
	/// <param name=""></param>
	CLinkedList(CLinkedListNode<T> Node)
	{
		if (Node != NULL)
		{
			RootNode = Node;
		}
		else
		{

		}
	}

	~CLinkedList()
	{

	}

public:

	void Push(CLinkedListNode<T> Node, CLinkedListNode** RootNode)
	{
		if (Node != nullptr)
		{
			
		}
	}


		


private:
	int Size;
	CLinkedListNode<T>* RootNode;
	CLinkedListNode<T>* EndNode;
};

