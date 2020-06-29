#ifndef BASE_IDE_HPP_INCLUDED
#define BASE_IDE_HPP_INCLUDED

#include <imguicolortextedit/TextEditor.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <streambuf>

namespace dcpu
{
    namespace detail
    {
        inline
        std::string read_file(const std::string& in)
        {
            std::ifstream t(in);
            std::string str;

            t.seekg(0, std::ios::end);
            str.reserve(t.tellg());
            t.seekg(0, std::ios::beg);

            str.assign((std::istreambuf_iterator<char>(t)),
                        std::istreambuf_iterator<char>());

            return str;
        }
    }

    inline
    nlohmann::json instruction_to_description()
    {
        static nlohmann::json data = nlohmann::json::parse(detail::read_file("./instructions.json"));

        return data;
    }

    /*inline
    std::string get_description_of_instruction(const std::string& in)
    {
        const std::vector<std::pair<std::string, std::string>>& mapping = instruction_to_description();

        //if(auto it = mapping.find(in); it != mapping.end())
        //    return it->second;

        for(auto& i : mapping)
        {
            if(i.first == in)
                return i.second;
        }

        return "No such instruction";
    }*/

    namespace ide
    {
        inline
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

        struct editor
        {
            static inline int gid = 0;
            int id = gid++;

            TextEditor edit;
            CPU c;
            stack_vector<uint16_t, MEM_SIZE> translation_map;
            bool halted = false;

            editor()
            {
                auto lang = TextEditor::LanguageDefinition::CPlusPlus();
                auto mapping = dcpu::instruction_to_description();

                for(auto& i : mapping)
                {
                    TextEditor::Identifier id;
                    id.mDeclaration = i["description"];
                    lang.mIdentifiers.insert(std::make_pair(i["name"], id));
                }

                edit.SetLanguageDefinition(lang);
            }

            void render()
            {
                ImGui::Begin((std::string("IDE") + std::to_string(id)).c_str());

                ImGui::BeginChild(("Child" + std::to_string(id)).c_str(), ImVec2(400, 0));

                {
                    std::unordered_set<int> current_pc;

                    int line = 0;
                    int seek_character = translation_map[c.regs[PC_REG]];

                    std::string seek = edit.GetText();

                    while(seek_character < seek.size() && should_prune(seek[seek_character]))
                        seek_character++;

                    for(int i=0; i <= seek_character && i < seek.size(); i++)
                    {
                        if(seek[i] == '\n')
                            line++;
                    }

                    current_pc.insert(line+1);

                    edit.SetBreakpoints(current_pc);
                }

                edit.Render((std::string("IDEW") + std::to_string(id)).c_str());

                ImGui::EndChild();

                ImGui::SameLine();

                ImGui::BeginGroup();

                if(ImGui::Button("Assemble"))
                {
                    auto [rinfo_opt, err] = assemble(edit.GetText());

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

                ImGui::End();
            }
        };

        struct reference_card
        {
            void render()
            {
                ImGui::Begin("Reference", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

                auto all_instr = dcpu::instruction_to_description();

                std::map<std::string, std::vector<nlohmann::json>> by_category;

                for(auto& i : all_instr)
                {
                    std::string category = i["category"];

                    by_category[category].push_back(i);
                }

                for(auto& [cat, vec] : by_category)
                {
                    if(ImGui::TreeNode(cat.c_str()))
                    {
                        for(nlohmann::json& data : vec)
                        {
                            std::string name = data["name"];
                            std::string desc = data["description"];

                            if(ImGui::TreeNode(name.c_str()))
                            {
                                ImGui::Text(desc.c_str());

                                ImGui::TreePop();
                            }
                            else if(ImGui::IsItemHovered())
                            {
                                ImGui::SetTooltip("%s", desc.c_str());
                            }
                        }

                        ImGui::TreePop();
                    }
                }

                ImGui::End();
            }
        };
    }
}

#endif // BASE_IDE_HPP_INCLUDED
