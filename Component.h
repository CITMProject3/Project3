#ifndef __COMPONENT_H__
#define __COMPONENT_H__

enum ComponentType
{
	C_TRANSFORM = 0,
	C_MESH = 1,
	C_MATERIAL = 2,
	C_CAMERA = 3,
	C_LIGHT = 4,
<<<<<<< HEAD
	C_AUDIO = 5,
	C_COLLIDER = 6,
	C_CAR = 7,
	C_SCRIPT = 8
=======
	C_ANIMATION = 5,
	C_BONE = 6,
	C_AUDIO = 7,
	C_COLLIDER = 8,
	C_CAR = 9,
>>>>>>> refs/remotes/origin/Engine
};

class GameObject;
class Data;

class Component
{
public:

	Component(ComponentType type, GameObject* game_object);
	virtual ~Component();


	virtual void Update() {};
	virtual void PreUpdate() {}
	virtual void PostUpdate() {}

	bool IsActive();
	void SetActive(bool value);
	virtual void OnInspector(bool debug) {}

	ComponentType GetType()const;
	GameObject* GetGameObject()const;
	unsigned int GetUUID()const;

	virtual void OnTransformModified() {}
	virtual void Save(Data& file) const {}
	virtual void Remove();
	virtual void Load(Data& config) {}

	virtual void OnPlay() {}
	virtual void OnPause() {}
	virtual void OnStop() {}

protected:
	bool active = true;
	ComponentType type;
	GameObject* game_object = nullptr;
	unsigned int uuid = 0;

};
#endif // !__COMPONENT_H__