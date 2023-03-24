#ifndef BASE_IDE_HPP_INCLUDED
#define BASE_IDE_HPP_INCLUDED

#include <imgui/imgui.h>
#include <dcpu16-sim/base_sim.hpp>
#include <string>
#include <chrono>

struct TextEditor;
struct MemoryEditor;

struct error_info;
struct return_info;

///TODO: https://github.com/ocornut/imgui_club/tree/master/imgui_memory_editor
namespace dcpu
{
    namespace ide
    {
        std::string format_error(const error_info& inf);

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

        struct project_instance_base;

        struct editor
        {
            std::string file_name;
            bool unsaved = false;
            TextEditor* edit = nullptr;
            MemoryEditor* memory_edit = nullptr;
            dcpu::sim::CPU c;
            stack_vector<uint16_t, MEM_SIZE> translation_map;
            stack_vector<uint16_t, MEM_SIZE> pc_to_source_line;
            stack_vector<uint16_t, MEM_SIZE> source_line_to_pc;
            bool halted = false;

            bool wants_step = false;
            bool wants_run = false;
            bool wants_pause = false;
            bool wants_assemble = false;
            bool wants_reset = false;
            bool wants_save = false;

            bool is_running = false;

            bool is_hex = false;
            bool is_sign = false;
            bool is_modifiable = false; ///registers are editable
            bool is_rendering_mem_editor = false;

            ///unsupported within the ide atm, being used for game. This is a TODO
            int clock_hz = 1000;
            bool dirty_frequency = false;

            std::string additional_info;

            bool dirty_errors = false;
            std::string error_string;
            int error_line = 0;

            editor();

            void render(project_instance_base& instance, int id);
            void render_inline(project_instance_base& instance, int id);

            void render_memory_editor_inline(project_instance_base& instance, int id);

            void handle_default_step();

            void assemble_from(const return_info& inf);
            void set_errors(const error_info& inf);

            virtual bool assemble();

            virtual bool step()
            {
                return c.step();
            }

            std::string get_text() const;
            void set_text(const std::string& str);

            virtual ~editor(){}
        };

        struct project_instance_base
        {
            project proj;

            std::chrono::time_point<std::chrono::steady_clock> autosave_timer = std::chrono::steady_clock::now();
            bool is_autosaving = true;

            virtual void load(const std::string& file){}
            virtual void save(){}

            virtual ~project_instance_base(){}
        };

        template<typename T = editor>
        struct project_instance : project_instance_base
        {
            std::vector<T> editors;

            virtual void load(const std::string& file) override
            {
                proj = project();
                proj.load(file);

                editors.clear();

                for(int i=0; i < (int)proj.assembly_data.size(); i++)
                {
                    T& edit = editors.emplace_back();
                    edit.set_text(proj.assembly_data.at(i));
                    edit.file_name = proj.assembly_files.at(i);
                }
            }

            virtual void save() override
            {
                assert(editors.size() == proj.assembly_data.size());

                for(int i=0; i < (int)editors.size(); i++)
                {
                    proj.assembly_data[i] = editors[i].get_text();

                    editors[i].unsaved = false;
                }

                proj.save();
            }
        };

        struct reference_card
        {
            void render();
            void render_inline();
        };
    }
}

#endif // BASE_IDE_HPP_INCLUDED
