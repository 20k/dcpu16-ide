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

///TODO: Communication channels
void register_editor(const std::string& name, uint16_t& val)
{
    int ival = val;

    ImGui::Text(name.c_str());

    ImGui::SameLine();

    ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());

    //ImGui::PushItemWidth(ImGui::CalcTextSize("65535").x*4 + 40);

    ImGui::InputInt(("##" + name).c_str(), &ival);

    val = ival;
}

int main()
{
    render_settings sett;
    sett.width = 1200;
    sett.height = 800;

    render_window win(sett, "DCPU16-IDE");

    bool halted = false;
    CPU c;
    dcpu::ide::editor edit;
    dcpu::ide::reference_card card;

    stack_vector<uint16_t, MEM_SIZE> translation_map;

    while(!win.should_close())
    {
        win.poll();

        ImGui::Begin("Hello");

        ImGui::BeginChild("Child", ImVec2(400, 0));

        {
            std::unordered_set<int> current_pc;

            int line = 0;
            int seek_character = translation_map[c.regs[PC_REG]];

            std::string seek = edit.edit.GetText();

            while(seek_character < seek.size() && should_prune(seek[seek_character]))
                seek_character++;

            for(int i=0; i <= seek_character && i < seek.size(); i++)
            {
                if(seek[i] == '\n')
                    line++;
            }

            current_pc.insert(line+1);

            edit.edit.SetBreakpoints(current_pc);
        }

        edit.render();
        card.render();

        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginGroup();

        if(ImGui::Button("Assemble"))
        {
            auto [rinfo_opt, err] = assemble(edit.edit.GetText());

            if(rinfo_opt.has_value())
            {
                c = CPU();
                c.load(rinfo_opt.value().mem, 0);
                halted = false;

                translation_map = rinfo_opt.value().translation_map;
            }
        }

        if(ImGui::Button("Step"))
        {
            if(!halted)
                halted = halted || c.step();
        }

        register_editor("A: ", c.regs[A_REG]);
        register_editor("B: ", c.regs[B_REG]);
        register_editor("C: ", c.regs[C_REG]);
        register_editor("X: ", c.regs[X_REG]);
        register_editor("Y: ", c.regs[Y_REG]);
        register_editor("Z: ", c.regs[Z_REG]);
        register_editor("I: ", c.regs[I_REG]);
        register_editor("J: ", c.regs[J_REG]);
        register_editor("PC:", c.regs[PC_REG]);
        register_editor("SP:", c.regs[SP_REG]);
        register_editor("EX:", c.regs[EX_REG]);
        register_editor("IA:", c.regs[IA_REG]);

        ImGui::EndGroup();

        //ImGui::Text("Hello");

        ImGui::End();

        sf::sleep(sf::milliseconds(1));

        win.display();
    }

    return 0;
}
