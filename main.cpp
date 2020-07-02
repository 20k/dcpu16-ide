#include <imgui/imgui.h>
#include <toolkit/render_window.hpp>
#include <vec/vec.hpp>
#include <dcpu16-sim/base_sim.hpp>
#include <dcpu16-asm/base_asm.hpp>
#include "base_ide.hpp"
#include <SFML/System.hpp>
#include <toml.hpp>

#ifdef __WIN32__
#include <windows.h>
#endif // __WIN32__

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

int main(int argc, char* argv[])
{
    for(int i=0; i < argc; i++)
    {
        printf("ARG: %s\n", argv[i]);
    }

    render_settings sett;
    sett.width = 1200;
    sett.height = 800;

    render_window win(sett, "DCPU16-IDE");

    //dcpu::ide::editor edit;
    dcpu::ide::reference_card card;

    dcpu::sim::fabric cpu_fabric;
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
                i.c.cycle_step(&cpu_fabric);
            }

            stack_vector<dcpu::sim::CPU*, 64> cpus;

            for(auto& i : cpu_count)
            {
                cpus.push_back(&i.c);
            }

            resolve_interprocessor_communication(cpus, cpu_fabric);
        }

        if(ImGui::Button("Assemble All"))
        {
            cpu_fabric = dcpu::sim::fabric();

            for(dcpu::ide::editor& edit : cpu_count)
            {
                auto [rinfo_opt, err] = assemble(edit.edit.GetText());

                if(rinfo_opt.has_value())
                {
                    edit.c = dcpu::sim::CPU();
                    edit.c.load(rinfo_opt.value().mem, 0);
                    edit.halted = false;

                    edit.translation_map = rinfo_opt.value().translation_map;
                }
                else
                {
                    printf("Err %s\n", err.data());
                }
            }
        }

        ImGui::End();

        sf::sleep(sf::milliseconds(1));

        win.display();
    }

    return 0;
}
