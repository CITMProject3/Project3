#ifndef __COMPONENT_H__
#define __COMPONENT_H__

enum ComponentType
{
	C_TRANSFORM = 0,
	C_MESH = 1,
	C_MATERIAL = 2,
	C_CAMERA = 3,
	C_LIGHT = 4,
	C_ANIMATION = 5,
	C_BONE = 6,
	C_AUDIO = 7,
	C_COLLIDER = 8,
	C_CAR = 9,
	C_RECT_TRANSFORM = 10,
	C_CANVAS = 11,
	C_UI_IMAGE = 12,
	C_UI_TEXT = 13,
	C_UI_BUTTON = 14
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