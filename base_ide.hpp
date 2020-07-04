#ifndef BASE_IDE_HPP_INCLUDED
#define BASE_IDE_HPP_INCLUDED

#include <imgui/imgui.h>
#include <dcpu16-sim/base_sim.hpp>
#include <string>

struct TextEditor;

namespace dcpu
{
    namespace ide
    {
        struct project
        {
            std::string project_file;
            std::vector<std::string> assembly_files;

            std::string project_data;
            std::vector<std::string> assembly_data;

            void load(const std::string& str);
            void save();
        };

        struct settings
        {
            std::vector<std::string> files;

            void load(const std::string& file);
        };

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

            TextEditor* edit = nullptr;
            dcpu::sim::CPU c;
            stack_vector<uint16_t, MEM_SIZE> translation_map;
            bool halted = false;

            editor();

            void render();

            std::string get_text() const;
            void set_text(const std::string& str);
        };

        struct project_instance
        {
            project proj;

            std::vector<editor> editors;

            void load(const std::string& file);
            void save();
        };

        struct reference_card
        {
            void render();
        };
    }
}

#endif // BASE_IDE_HPP_INCLUDED
