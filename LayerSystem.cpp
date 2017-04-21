#include "LayerSystem.h"
#include "Globals.h"
#include "imgui\imgui.h"
#include <bitset>

LayerSystem::LayerSystem()
{
	layers.resize(MAX_LAYERS);
	layers[0] = "Default";
}

LayerSystem::~LayerSystem()
{
}

bool LayerSystem::AddLayer(unsigned int index, const std::string& name)
{
	if (index >= MAX_LAYERS || index == 0)
		return false;

	layers[index] = name;
	return true;
}

std::vector<std::string>* LayerSystem::GetEditableLayerList() 
{
	return &layers;
}

void LayerSystem::Load(Data & data)
{
	
	if (data.GetArray("layers", 0).IsNull())
	{
		LOG("Layers configuration could not be loaded.");
		return;
	}

	for (int i = 0; i < MAX_LAYERS; ++i)
	{
		if (i == 0)
			continue;
		Data layer_data;
		layer_data = data.GetArray("layers", i);
		layers[i] = layer_data.GetString("name");
	}
}

void LayerSystem::Save(Data & data) const
{
	for (int i = 0; i < MAX_LAYERS; ++i)
	{
		Data layer;
		layer.AppendString("name", layers[i].data());
		data.AppendArrayValue(layer);
	}
}

void LayerSystem::DisplayLayerMask(int& value)
{
	int bin_value = value;

	bool everything = false, nothing = false;

	if (value == -1)
	{
		everything = true;
	}

	if (value == 0)
	{
		nothing = true;
	}

	if (ImGui::BeginMenu("Layers: "))
	{
		if (ImGui::MenuItem("Nothing", NULL, nothing))
		{
			value = 0;
		}

		if (ImGui::MenuItem("Everything", NULL, everything))
		{
			value = -1;
		}

		for (int i = 0; i < MAX_LAYERS; ++i)
		{
			if (layers[i].size() != 0)
			{
				bool enabled = false;
				
				if (everything)
				{
					enabled = true;
				}
				else
				{
					if(!nothing)
						enabled = (bin_value >> i) & 1;
				}
				
				
				if (ImGui::MenuItem(layers[i].data(), NULL, enabled))
				{
					int decimal_value = Pow(2, i);
					if (enabled == false)
						value += decimal_value;
					else
						value -= decimal_value;
				}
			}
		}

		ImGui::EndMenu();
	}
	
}

void LayerSystem::DisplayLayerSelector(int & value)
{
	int tmp_value = value;
	if (value < 0 || value >= layers.size())
	{
		return;
	}

	std::string menu_name = "Layer: " + layers[value];
	if (ImGui::BeginMenu(menu_name.data()))
	{
		for (int i = 0; i < MAX_LAYERS; ++i)
		{
			if (layers[i].size() != 0)
			{
				bool enabled = (tmp_value == i) ? true : false;

				if (ImGui::MenuItem(layers[i].data(), NULL, enabled))
				{
					value = i;
				}
			}
		}

		ImGui::EndMenu();
	}
}

int LayerSystem::BinaryToDecimal(int value)
{
	int ret = 0;
	int i = 0;
	int remainder;

	while (value != 0)
	{
		remainder = value % 10;
		value /= 10;
		ret += remainder * Pow(2, i);
		++i;
	}
	return ret;
}

int LayerSystem::DecimalToBinary(int value)
{
	int ret = 0;
	int remainder;
	int i = 1;

	while (value != 0)
	{
		remainder = value % 2;
		value /= 2;
		ret += remainder*i;
		i *= 10;
	}
	return ret;
}
