#ifndef __COMPONENT_H__
#define __COMPONENT_H__

enum ComponentType
{
	C_TRANSFORM = 0,
	C_MESH = 1,
	C_MATERIAL = 2,
	C_CAMERA = 3,
	C_LIGHT = 4,
	C_AUDIO = 5,
	C_COLLIDER,
	C_CAR,
	C_RECT_TRANSFORM,
	C_CANVAS,
	C_UI_IMAGE,
	C_UI_TEXT
};

class GameObject;
class Data;

class Component
{
public:
	
	Component(ComponentType type, GameObject* game_object);
	virtual ~Component();

	virtual void PreUpdate() {}
	virtual void Update() {}
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