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
#include <dcpu16-sim/base_sim.hpp>
#include <dcpu16-asm/base_asm.hpp>
#include "base_ide.hpp"
#include <SFML/System.hpp>
#include <unordered_set>

/*SET X, 10

:loop
SET Y, X
SUB X, 1
IFA X, 0
SET PC, loop
SET Z, 1*/

/*SET X, 10

:loop

SET Y, X
SUB X, 1

IFA X, 0


SET PC, loop
SET Z, 1*/

/*SET X, 10

:loop

SET Y, X
SUB X, 1
SET Z 6553

IFA X, 0


SET PC, loop
SET Z, 1*/

///TODO: Communication channelss
int main()
{
    render_settings sett;
    sett.width = 1200;
    sett.height = 800;

    render_window win(sett, "DCPU16-IDE");

    //dcpu::ide::editor edit;
    dcpu::ide::reference_card card;

    std::vector<dcpu::ide::editor> cpu_count;
    cpu_count.emplace_back();

    while(!win.should_close())
    {
        win.poll();

        card.render();
        //edit.render();

        for(auto& i : cpu_count)
        {
            i.render();
        }

        ImGui::Begin("Control Panel", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        int num_cpus = cpu_count.size();

        ImGui::InputInt("CPUs", &num_cpus);

        if(num_cpus < 0)
            num_cpus = 0;

        cpu_count.resize(num_cpus);

        if(ImGui::Button("Step All"))
        {
            for(auto& i : cpu_count)
            {
                i.c.cycle_step();
            }

            stack_vector<CPU*, 64> cpus;

            for(auto& i : cpu_count)
            {
                cpus.push_back(&i.c);
            }

            resolve_interprocessor_communication(cpus);
        }

        ImGui::End();

        sf::sleep(sf::milliseconds(1));

        win.display();
    }

    return 0;
}
