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
    }
}

#endif // BASE_IDE_HPP_INCLUDED
