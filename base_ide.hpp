#ifndef BASE_IDE_HPP_INCLUDED
#define BASE_IDE_HPP_INCLUDED

#include <imgui/imgui.h>
#include <dcpu16-sim/base_sim.hpp>
#include <string>

struct TextEditor;

///TODO: https://github.com/ocornut/imgui_club/tree/master/imgui_memory_editor
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

        struct project_instance;

        struct editor
        {
            bool unsaved = false;
            TextEditor* edit = nullptr;
            dcpu::sim::CPU c;
            stack_vector<uint16_t, MEM_SIZE> translation_map;
            stack_vector<uint16_t, MEM_SIZE> pc_to_source_line;
            bool halted = false;

            std::string additional_info;

            editor();

            void render(project_instance& instance, int id);

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
