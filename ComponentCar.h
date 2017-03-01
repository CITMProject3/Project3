#ifndef _COMPONENT_CAR_H_
#define _COMPONENT_CAR_H_

#include "Component.h"



class ComponentCar : public Component
{
public:

	//
	ComponentCar(GameObject* GO);
	~ComponentCar();

	//Inherited Component Methods
	void Update();

	void OnTransformModified();
	void Save(Data& file) const;
	void Remove();
	void Load(Data& config);

	void OnInspector(bool debug);

	//Input handler during Game (import in the future to scripting)
	void HandlePlayerInput();


};


#endif // !_COMPONENT_CAR_H_

