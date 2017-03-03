#include "CurveWindow.h"

#include "Application.h"
#include "Editor.h"
#include "Hierarchy.h"

CurveWindow::CurveWindow()
{
	flags |= ImGuiWindowFlags_MenuBar;
	ResetPoints();
}

CurveWindow::~CurveWindow() {}

void CurveWindow::Draw()
{
	if (!active)
		return;

	ImGui::Begin("Curve Window", &active, flags);

	if (ImGui::RadioButton("Create points", mode == ImGuiCurveEditorMode_CreatePoints))
	{
		mode = ImGuiCurveEditorMode_CreatePoints;
	}

	if (ImGui::RadioButton("Edit points", mode == ImGuiCurveEditorMode_EditPoints))
	{
		mode = ImGuiCurveEditorMode_EditPoints;
	}

	if (ImGui::RadioButton("Edit tangents", mode == ImGuiCurveEditorMode_EditTangents))
	{
		mode = ImGuiCurveEditorMode_EditTangents;
	}

	if (ImGui::Curve("Curve Editor", ImVec2(600, 200), points, mode, axis))
	{
		// curve changed
	}

	ImGui::Text("Values");
	ImGui::Separator();
	
	std::vector<std::string> titles;
	titles.push_back("Axis X");
	titles.push_back("Axis Y");
	titles.push_back("Axis Z");

	for (uint aX = 0; aX < 3; aX++)
	{
		ImGui::Text(titles[aX].c_str());
		for (uint i = 0; i < points.size(); i++)
		{
			std::string id = "##" + std::to_string(i) + std::to_string(aX);
			float value[2] = { points[aX][i].x, points[aX][i].y };
			if (ImGui::DragFloat2(id.c_str(), value))
			{
				points[aX][i].x = value[0];
				points[aX][i].y = value[1];
			}
		}
		if (aX < 3)
			ImGui::Separator();
	}


	if (ImGui::Button("Add Point"))
	{
		//points.push_back(ImVec2(1, 0.5f));
	}

	if (ImGui::Button("Reset points"))
	{
		ResetPoints();
	}

	//float value_you_care_about = ImGui::CurveValue(0.7f, 10, points); // calculate value at position 0.7

	ImGui::End();
}

void CurveWindow::ResetPoints()
{
	points.clear();

	std::vector<ImVec2> axisX;
	axisX.push_back(ImVec2(0.0, 0.1));
	axisX.push_back(ImVec2(0.33, 0.2));
	axisX.push_back(ImVec2(0.66, 0.1));
	axisX.push_back(ImVec2(1.0, 0.2));

	std::vector<ImVec2> axisY;
	axisY.push_back(ImVec2(0.0, 0.4));
	axisY.push_back(ImVec2(0.33, 0.5));
	axisY.push_back(ImVec2(0.66, 0.4));
	axisY.push_back(ImVec2(1.0, 0.5));

	std::vector<ImVec2> axisZ;
	axisZ.push_back(ImVec2(0.0, 0.8));
	axisZ.push_back(ImVec2(0.33, 0.9));
	axisZ.push_back(ImVec2(0.66, 0.8));
	axisZ.push_back(ImVec2(1.0, 0.9));

	points.push_back(axisX);
	points.push_back(axisY);
	points.push_back(axisZ);
}