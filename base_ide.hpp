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
        struct editor
        {
            TextEditor edit;

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
                edit.Render("IDE");
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
