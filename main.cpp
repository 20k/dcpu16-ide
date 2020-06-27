#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/examples/imgui_impl_glfw.h>
#include <imgui/examples/imgui_impl_opengl3.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
//#include "window_context.hpp"
#include <toolkit/render_window.hpp>
#include <vec/vec.hpp>
#include <imguicolortextedit/TextEditor.h>
#include "base_ide.hpp"

int main()
{
    render_settings sett;
    sett.width = 1200;
    sett.height = 1000;

    render_window win(sett, "DCPU16-IDE");

    dcpu::ide::editor edit;

    while(!win.should_close())
    {
        win.poll();

        ImGui::Begin("Hello", nullptr, ImGuiWindowFlags_MenuBar);

        ImGui::BeginChild("Child", ImVec2(400, 0));

        if(ImGui::BeginMenuBar())
        {
            if(ImGui::BeginMenu("Reference"))
            {
                //ImGui::MenuItem("Test");

                auto all_instr = dcpu::instruction_to_description();

                /*for(auto i : all_instr)
                {
                    std::string name = i["name"];
                    std::string desc = i["description"];

                    if(ImGui::MenuItem(name.c_str()))
                    {
                        ImGui::SetTooltip(desc.c_str());
                    }
                }*/

                std::map<std::string, std::vector<nlohmann::json>> by_category;

                for(auto& i : all_instr)
                {
                    std::string category = i["category"];

                    by_category[category].push_back(i);
                }

                for(auto& i : by_category)
                {
                    if(ImGui::BeginMenu(i.first.c_str()))
                    {
                        for(nlohmann::json& dat : i.second)
                        {
                            std::string name = dat["name"];
                            std::string desc = dat["description"];

                            ImGui::MenuItem(name.c_str());

                            if(ImGui::IsItemHovered())
                            {
                                ImGui::BeginTooltip();
                                ImGui::TextUnformatted(desc.c_str());
                                ImGui::EndTooltip();
                            }
                        }

                        ImGui::EndMenu();
                    }
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        edit.render();

        ImGui::EndChild();

        ImGui::SameLine();

        if(ImGui::Button("Step"))
        {

        }

        //ImGui::Text("Hello");

        ImGui::End();

        win.display();
    }

    return 0;
}
