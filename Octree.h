#ifndef __OCTREE_H__
#define __OCTREE_H__

#include "MathGeoLib\include\MathGeoLib.h"

#include "DebugDraw.h"

#include <vector>
#include <stack>
#include <queue>

template<typename Type> class Octree;

template<typename Type>
class OctreeNode
{
	friend class Octree<Type>;
public:
	OctreeNode();
	OctreeNode(OctreeNode<Type>* parent, const math::AABB& bbox);
	~OctreeNode();

private:

	void Divide();

private:
	math::AABB bbox;
	Type content;
	math::float3 position;
	OctreeNode<Type>* parent = nullptr;
	std::vector<OctreeNode*> childs;
	OctreeNode<Type>* next = nullptr;
	bool empty = true;
};

template<typename Type>
class Octree
{
public:
	Octree();
	~Octree();

	bool Insert(Type object, const math::float3& position, OctreeNode<Type>* start_from = nullptr);
	bool Remove(Type object, const math::float3& position);
	void Create(float size);
	void Draw();
	
	template<typename Primitive>
	bool Intersect(vector<Type>& results, Primitive& prim)const;
private:
	void RemoveUnnecessaryNodes(OctreeNode<Type>* start);

private:
	OctreeNode<Type>* root = nullptr;
};

// OCTREE NODE ------------------------------------------------------------------------------------------------------------------------------
template<typename Type>
inline OctreeNode<Type>::OctreeNode()
{}

template<typename Type>
inline OctreeNode<Type>::OctreeNode(OctreeNode<Type>* parent, const math::AABB& bbox) : parent(parent), bbox(bbox)
{}

template<typename Type>
inline OctreeNode<Type>::~OctreeNode()
{}

template<typename Type>
inline void OctreeNode<Type>::Divide()
{
	AABB bbox = this->bbox;
	OctreeNode<Type>* previous = nullptr;
	float3 center = this->bbox.CenterPoint();
	float half = this->bbox.HalfSize().x; //Same size at X/Y/Z
	/*
		---------	 ---------
		| 0 | 1 |	 | 4 | 5 |
		---------	 ---------
		| 2 | 3 |	 | 6 | 7 |
		---------	 ---------
		bottom			top
	
	*/
	for (int i = 0; i < 8; i++)
	{
		switch (i)
		{
		case 0:
			bbox.minPoint = this->bbox.minPoint + float3(0, 0, half);
			bbox.maxPoint = center + float3(0, 0, half);
			break;
		case 1:
			bbox.minPoint = this->bbox.minPoint + float3(half, 0, half);
			bbox.maxPoint = center + float3(half, 0, half);
			break;
		case 2:
			bbox.minPoint = this->bbox.minPoint;
			bbox.maxPoint = center;
			break;
		case 3:
			bbox.minPoint = this->bbox.minPoint + float3(half, 0, 0);
			bbox.maxPoint = center + float3(half, 0, 0);
			break;
		case 4:
			bbox.minPoint = center - float3(half, 0, 0);
			bbox.maxPoint = this->bbox.maxPoint - float3(half, 0, 0);
			break;
		case 5:
			bbox.minPoint = center;
			bbox.maxPoint = this->bbox.maxPoint;
			break;
		case 6:
			bbox.minPoint = center - float3(half, 0, half);
			bbox.maxPoint = this->bbox.maxPoint - float3(half, 0, half);
			break;
		case 7:
			bbox.minPoint = center - float3(0, 0, half);
			bbox.maxPoint = this->bbox.maxPoint - float3(0, 0, half);
			break;
		}

		OctreeNode<Type>* node = new OctreeNode<Type>(this, bbox);
		
		if (previous)
			previous->next = node;

		previous = node;
		childs.push_back(node);
	}
}


// OCTREE ------------------------------------------------------------------------------------------------------------------------------
template<typename Type>
inline Octree<Type>::Octree()
{}

template<typename Type>
inline Octree<Type>::~Octree()
{
	if (root)
		delete root;
}

template<typename Type>
inline bool Octree<Type>::Insert(Type object, const math::float3& position, OctreeNode<Type>* start_from)
{
	OctreeNode<Type>* start_node = (start_from) ? start_from : root;

	if (root == nullptr) //Octree has not been created
		return false;

	if (start_node->bbox.Contains(position) == false) //Point out of the boundaries
		return false; 

	bool ret = false;
	queue<OctreeNode<Type>*> queue;
	OctreeNode<Type>* current = nullptr;

	queue.push(start_node);

	while (queue.empty() == false)
	{
		current = queue.front();
		queue.pop();

		if (current->bbox.Contains(position))
		{
			if (current->childs.size() == 0) //Leaf node ->Insertion
			{
				if (current->empty)
				{
					current->content = object;
					current->position = position;
					current->empty = false;
					ret = true; //Successful insertion!
					break;
				}
				else
				{
					current->Divide();
					current->empty = true;

					//Insert current content
					ret = Insert(current->content, current->position, current);
					if (ret == false) //Error while inserting current content
						break;
					//Insert new content
					ret = Insert(object, position, current); //If ret == true is a successful insertion!
					break;
				}
			}
		}

		for (vector<OctreeNode<Type>*>::iterator child = current->childs.begin(); child != current->childs.end(); ++child)
			queue.push(*child);

	}

	return ret;
}

template<typename Type>
inline bool Octree<Type>::Remove(Type object, const math::float3 & position)
{
	if (root == nullptr) //Octree has not been created
		return false;

	if (root->bbox.Contains(position) == false) //Point out of the boundaries
		return false;

	bool ret = false;
	queue<OctreeNode<Type>*> queue;
	OctreeNode<Type>* current = nullptr;

	queue.push(root);

	while (queue.empty() == false)
	{
		current = queue.front();
		queue.pop();

		if (current->bbox.Contains(position))
		{
			if (current->childs.size() == 0) //Leaf node
			{
				if (!current->empty && current->content == object && current->position.Equals(position))
				{
					current->empty = true;
					RemoveUnnecessaryNodes(current);
					ret = true; //Successful erase!
					break;
				}
			}
		}

		for (vector<OctreeNode<Type>*>::iterator child = current->childs.begin(); child != current->childs.end(); ++child)
			queue.push(*child);

	}

	return ret;
}

template<typename Type>
inline void Octree<Type>::Create(float size)
{
	if (root)
		delete root; 
	math::AABB bbox = math::AABB(math::vec(-(size / 2.0f)), math::vec(size / 2.0f));
	root = new OctreeNode<Type>(nullptr, bbox);
}

template<typename Type>
inline void Octree<Type>::Draw()
{
	if (root == nullptr)
		return;

	stack<OctreeNode<Type>*> stack;
	OctreeNode<Type>* current = root;
	OctreeNode<Type>* print_item;
	while (current != nullptr || stack.size() != 0)
	{
		if (current == nullptr)
		{
			print_item = stack.top();
			stack.pop();
			g_Debug->AddAABB(print_item->bbox, g_Debug->red, 3.0f);
			current = print_item->next;
		}
		else
		{
			stack.push(current);
			if (current->childs.size() > 0)
				current = current->childs.front();
			else
				current = nullptr;
		}
	}	
}

template<typename Type>
inline void Octree<Type>::RemoveUnnecessaryNodes(OctreeNode<Type>* start)
{
	if (start == root)
		return;

	OctreeNode<Type>* parent = start->parent;
	Type content;
	float3 pos;
	while (parent != nullptr)
	{
		int num_child_with_content = 0;
		for (vector<OctreeNode<Type>*>::iterator child = parent->childs.begin(); child != parent->childs.end(); ++child)
		{
			if ((*child)->empty == false)
			{
				++num_child_with_content;
				content = (*child)->content;
				pos = (*child)->position;
			}
		}

		if (num_child_with_content == 1)
		{
			parent->childs.clear();
			parent->content = content;
			parent->position = pos;
			parent->empty = false;
			parent = parent->parent;
		}
		else
			break;
	}
}

template<typename Type>
template<typename Primitive>
inline bool Octree<Type>::Intersect(vector<Type>& results, Primitive & prim) const
{
	if (root == nullptr) //Octree has not been created
		return false;

	if (!prim.Intersects(root->bbox)) //Point out of the boundaries
		return false;

	bool ret = false;
	queue<OctreeNode<Type>*> queue;
	OctreeNode<Type>* current = nullptr;

	queue.push(root);

	while (queue.empty() == false)
	{
		current = queue.front();
		queue.pop();

		if (prim.Intersects(current->bbox))
		{
			if (current->childs.size() == 0) //Leaf node
			{
				if (current->empty == false)
				{
					results.push_back(current->content);
					ret = true;
				}
			}
			else
				for (vector<OctreeNode<Type>*>::iterator child = current->childs.begin(); child != current->childs.end(); ++child)
					queue.push(*child);
		}
	}
	return ret; 
}

#endif // !__OCTREE_H__
