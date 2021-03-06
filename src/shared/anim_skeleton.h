#pragma once
#include "anim_joint.h"
#include "t_list.h"
#include "iterator_utils.h"

class Skeleton
{
private:
	int _nextJointID;

	List<Joint> _joints;

	bool _CanAddJoint(const Joint *parent) const
	{
		if (parent == nullptr)
			return true;

		for (const Joint& joint : _joints)
			if (&joint == parent)
				return true;

		return false;
	}

public:
	Skeleton() : _nextJointID(0) {}

	Skeleton(const Skeleton& other) : _nextJointID(other._nextJointID), _joints(other._joints) {}

	Skeleton(Skeleton&& other) noexcept : _nextJointID(other._nextJointID), _joints(other._joints)
	{
		other._nextJointID = 0;
		other.Clear();
	}

	~Skeleton() {}

	Skeleton& operator=(const Skeleton& other)
	{
		_nextJointID = other._nextJointID;
		_joints = other._joints;

		return *this;
	}

	Skeleton& operator=(Skeleton&& other) noexcept
	{
		_nextJointID = other._nextJointID;
		other._nextJointID = 0;

		_joints = std::move(other._joints);

		return *this;
	}

	void Clear() { _joints.Clear(); }

	size_t GetJointCount() const { return _joints.GetSize(); }

	List<Joint>::Iterator FirstListElement() { return _joints.begin(); }
	List<Joint>::ConstIterator FirstListElement() const { return _joints.begin(); }

	Joint* CreateJoint(Joint *parent)
	{
		if (_CanAddJoint(parent))
			return &_joints.AddBack(Joint(_nextJointID++, parent));

		return nullptr;
	}

	Joint* GetJointWithID(int id)
	{
		auto it = IteratorUtils::Offset(_joints.begin(), id);

		return it ? &*it : nullptr;
	}

	Joint* GetJointWithName(const String& name)
	{
		for (Joint& joint : _joints)
			if (joint.name == name)
				return &joint;

		return nullptr;
	}

	const Joint* GetJointWithID(int id) const				{ return (const Joint*)const_cast<Skeleton*>(this)->GetJointWithID(id); }
	const Joint* GetJointWithName(const String& name) const { return (const Joint*)const_cast<Skeleton*>(this)->GetJointWithName(name); }
};
