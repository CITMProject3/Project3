#include "TestWindow.h"

#include "Application.h"
#include "Editor.h"
#include "Hierarchy.h"

TestWindow::TestWindow()
{
	hierarchy = new Hierarchy();
}

TestWindow::~TestWindow(){}

void TestWindow::Draw(ImGuiWindowFlags flags)
{
	if (!active)
		return;

	ImGui::Begin("Splitter test", &active);
	int sizeX = ImGui::GetWindowSize().x;

	static float w = 100.0f;
	static float w2 = 100.0f;
	static float h = 300.0f;

	float w3 = sizeX - w - w2 - 8 * 4;

	float newW = w;
	float newW2 = w2;

	//ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	ImGui::BeginChild("child1", ImVec2(w, h), true);
	ImGui::Text("%f", w);
	ImGui::Separator();
//	ImGui::PopStyleVar();
//	hierarchy->Draw(flags);
//	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::InvisibleButton("vsplitter", ImVec2(8.0f, h));

	if (ImGui::IsItemActive())
	{
		newW += ImGui::GetIO().MouseDelta.x;
		newW2 -= ImGui::GetIO().MouseDelta.x;
	}

	ImGui::SameLine();
	ImGui::BeginChild("child2", ImVec2(w2, h), true);
	ImGui::Text("%f", w2);
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::InvisibleButton("vsplitter2", ImVec2(8.0f, h));
	if (ImGui::IsItemActive())
	{
		newW2 += ImGui::GetIO().MouseDelta.x;
	}


	ImGui::SameLine();
	ImGui::BeginChild("child3", ImVec2(w3, h), true);
	ImGui::Text("%f", w3);
	ImGui::EndChild();

	ImGui::InvisibleButton("hsplitter", ImVec2(-1, 8.0f));
	if (ImGui::IsItemActive())
		h += ImGui::GetIO().MouseDelta.y;
	ImGui::BeginChild("child4", ImVec2(0, 0), true);
	ImGui::Text("Child4");
	ImGui::Text("Window Size = %f", sizeX);

	ImGui::EndChild();
//	ImGui::PopStyleVar();

	ImGui::End();

	w = newW;
	w2 = newW2;
}