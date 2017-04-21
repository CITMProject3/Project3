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
	//C_Audio was here
	C_COLLIDER = 8,
	C_CAR = 9,
	C_SCRIPT = 10,
	C_RECT_TRANSFORM = 11,
	C_CANVAS = 12,
	C_UI_IMAGE = 13,
	C_UI_TEXT = 14,
	C_UI_BUTTON = 15,
	C_GRID = 16,
	C_AUDIO_LISTENER = 17,
	C_AUDIO_SOURCE = 18,
	C_SPRITE = 19,
	C_PARTICLE_SYSTEM = 20
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
	virtual void OnFocus() {}
protected:
	bool active = true;
	ComponentType type;
	GameObject* game_object = nullptr;
	unsigned int uuid = 0;

};
#endif // !__COMPONENT_H__