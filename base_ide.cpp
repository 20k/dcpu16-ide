#include "base_ide.hpp"

#include <imguicolortextedit/TextEditor.h>
#include <nlohmann/json.hpp>
#include <dcpu16-asm/base_asm_fwd.hpp>
#include <toml.hpp>
#include <toolkit/fs_helpers.hpp>
#include <iostream>
#include <GLFW/glfw3.h>
#include <filesystem>

std::string dcpu::ide::format_error(const error_info& err)
{
    std::string ret = "Line: ";

    ///there's no string + string_view in c++, but you can .append
    ret.append(std::to_string(err.line+1));
    ret.append("\nMsg: ");
    ret.append(err.msg);
    ret.append("\nName: ");
    ret.append(err.name_in_source);

    return ret;
}

void dcpu::ide::project::load(const std::string& str)
{
    project_file = str;

    project_data = file::read(project_file, file::mode::TEXT);

    std::filesystem::path path = project_file;

    std::filesystem::path local_directory = path.remove_filename();

    std::istringstream is(project_data, std::ios_base::binary | std::ios_base::in);

    toml::value val = toml::parse(is, project_file);

    assembly_files = toml::get<std::vector<std::string>>(val["files"]);

    assembly_data.clear();

    for(auto& i : assembly_files)
    {
        assembly_data.push_back(file::read((local_directory / i).string(), file::mode::TEXT));
    }
}

void dcpu::ide::project::save()
{
    file::rename(project_file, project_file + ".tempproj");

    project_data = file::read(project_file + ".tempproj", file::mode::TEXT);

    toml::value val;

    try
    {
        val = toml::parse(project_data);
    }
    catch(...)
    {

    }

    val["files"] = assembly_files;

    std::string as_str = toml::format(val);

    file::write_atomic(project_file, as_str, file::mode::TEXT);

    file::remove(project_file + ".tempproj");

    assert(assembly_files.size() == assembly_data.size());

    std::filesystem::path path = project_file;
    std::filesystem::path local_directory = path.remove_filename();

    for(int i=0; i < (int)assembly_files.size(); i++)
    {
        file::write_atomic((local_directory / assembly_files[i]).string(), assembly_data[i], file::mode::TEXT);
    }
}

void dcpu::ide::project_instance::load(const std::string& file)
{
    proj = project();
    proj.load(file);

    editors.clear();

    for(int i=0; i < (int)proj.assembly_data.size(); i++)
    {
        dcpu::ide::editor& edit = editors.emplace_back();
        edit.set_text(proj.assembly_data[i]);
    }
}

void dcpu::ide::project_instance::save()
{
    assert(editors.size() == proj.assembly_data.size());

    for(int i=0; i < (int)editors.size(); i++)
    {
        proj.assembly_data[i] = editors[i].get_text();

        editors[i].unsaved = false;
    }

    proj.save();
}

nlohmann::json instruction_to_description()
{
    static nlohmann::json data = nlohmann::json::parse(
        R"end(
        [
{"name":"SET", "cycle":1, "class":"a", "opcode":1, "category":"arithmetic", "signed":false, "description":"SET b, a : Sets b to a"},
{"name":"ADD", "cycle":2, "class":"a", "opcode":2, "category":"arithmetic", "signed":false, "description":"ADD b, a : Sets b to b+a, sets EX to 0x0001 on overflow"},
{"name":"SUB", "cycle":2, "class":"a", "opcode":3, "category":"arithmetic", "signed":false, "description":"SUB b, a : Sets b to b-a, sets EX to 0xffff on underflow"},
{"name":"MUL", "cycle":2, "class":"a", "opcode":4, "category":"arithmetic", "signed":false, "description":"MUL b, a : Sets b to b * a, sets EX to ((b*a)>>16)&0xffff. (b, a) are unsigned"},
{"name":"MLI", "cycle":2, "class":"a", "opcode":5, "category":"arithmetic", "signed":true, "description":"MUL b, a : Sets b to b * a, sets EX to ((b*a)>>16)&0xffff. (b, a) are signed"},
{"name":"DIV", "cycle":3, "class":"a", "opcode":6, "category":"arithmetic", "signed":false, "description":"DIV b, a : Sets b to b/a, sets EX to ((b<<16)/a)&0xffff. If a == 0, sets b and EX to 0. (b, a) are unsigned"},
{"name":"DVI", "cycle":3, "class":"a", "opcode":7, "category":"arithmetic", "signed":true, "description":"DVI b, a : Like DIV. (b, a) are signed. Rounds towards 0"},
{"name":"MOD", "cycle":3, "class":"a", "opcode":8, "category":"arithmetic", "signed":false, "description":"MOD b, a : Sets b to b%a. If a==0, sets b to 0 instead"},
{"name":"MDI", "cycle":3, "class":"a", "opcode":9, "category":"arithmetic", "signed":true, "description":"MDI b, a : Like MOD. (b, a) are signed"},
{"name":"AND", "cycle":1, "class":"a", "opcode":10, "category":"logical", "signed":false, "description":"AND b, a : Sets b to b&a"},
{"name":"BOR", "cycle":1, "class":"a", "opcode":11, "category":"logical", "signed":false, "description":"BOR b, a : Sets b to b|a"},
{"name":"XOR", "cycle":1, "class":"a", "opcode":12, "category":"logical", "signed":false, "description":"XOR b, a : Sets b to b^a"},
{"name":"SHR", "cycle":1, "class":"a", "opcode":13, "category":"logical", "signed":false, "description":"SHR b, a : Sets b to b>>>a, sets EX to ((b<<16)>>a)&0xffff. Logical shift"},
{"name":"ASR", "cycle":1, "class":"a", "opcode":14, "category":"logical", "signed":false, "description":"ASR b, a : Sets b to b>>a, sets EX to ((b<<16)>>>a)&0xffff. Arithmetic shift. (b) is signed"},
{"name":"SHL", "cycle":1, "class":"a", "opcode":15, "category":"logical", "signed":false, "description":"SHL b, a : Sets b to b<<a, sets EX to ((b<<a)>>16)&0xffff"},
{"name":"IFB", "cycle":2, "class":"a", "opcode":16, "category":"control", "signed":false, "description":"IFB b, a : Performs next instruction only if (b&a)!=0"},
{"name":"IFC", "cycle":2, "class":"a", "opcode":17, "category":"control", "signed":false, "description":"IFC b, a : Performs next instruction only if (b&a)==0"},
{"name":"IFE", "cycle":2, "class":"a", "opcode":18, "category":"control", "signed":false, "description":"IFE b, a : Performs next instruction only if b==a"},
{"name":"IFN", "cycle":2, "class":"a", "opcode":19, "category":"control", "signed":false, "description":"IFN b, a : Performs next instruction only if b!=a"},
{"name":"IFG", "cycle":2, "class":"a", "opcode":20, "category":"control", "signed":false, "description":"IFG b, a : Performs next instruction only if b>a"},
{"name":"IFA", "cycle":2, "class":"a", "opcode":21, "category":"control", "signed":true, "description":"IFA b, a : Performs next instruction only if b>a (signed)"},
{"name":"IFL", "cycle":2, "class":"a", "opcode":22, "category":"control", "signed":false, "description":"IFL b, a : Performs next instruction only if b<a"},
{"name":"IFU", "cycle":2, "class":"a", "opcode":23, "category":"control", "signed":true, "description":"IFU b, a : Performs next instruction only if b<a (signed)"},
{"name":"ADX", "cycle":3, "class":"a", "opcode":26, "category":"arithmetic", "signed":false, "description":"ADX b, a : Sets b to b+a+EX, sets EX to 0x0001 on overflow, otherwise 0x0"},
{"name":"SBX", "cycle":3, "class":"a", "opcode":27, "category":"arithmetic", "signed":false, "description":"SBX b, a : Sets b to b-a+EX, sets EX to 0xffff on overflow, otherwise 0x0"},
{"name":"RCV", "cycle":4, "class":"a", "opcode":28, "category":"multiprocessor", "signed":false, "description":"RCV b, a : Receives some value in b on channel a, blocks until received"},
{"name":"SND", "cycle":4, "class":"a", "opcode":29, "category":"multiprocessor", "signed":false, "description":"SND b, a : Sends the value b on channel a, blocks until sent"},
{"name":"STI", "cycle":2, "class":"a", "opcode":30, "category":"arithmetic", "signed":false, "description":"STI b, a : Sets b to a, then increases I and J by 1"},
{"name":"STD", "cycle":2, "class":"a", "opcode":31, "category":"arithmetic", "signed":false, "description":"STD b, a : Sets b to a, then decreases I and J by 1"},
{"name":"JSR", "cycle":3, "class":"b", "opcode":1, "category":"stack", "signed":false, "description":"JSR a : Pushes the address of the next instruction to the stack, then sets PC to a"},
{"name":"INT", "cycle":4, "class":"b", "opcode":8, "category":"interrupt", "signed":false, "description":"INT a : Triggers a software interrupt with message a"},
{"name":"IAG", "cycle":1, "class":"b", "opcode":9, "category":"interrupt", "signed":false, "description":"IAG a : Sets a to IA"},
{"name":"IAS", "cycle":1, "class":"b", "opcode":10, "category":"interrupt", "signed":false, "description":"IAS a : Sets IA to a"},
{"name":"RFI", "cycle":3, "class":"b", "opcode":11, "category":"stack", "signed":false, "description":"RFI a : Disables interrupt queueing, pops A from the stack, then pops PC from the stack"},
{"name":"IAQ", "cycle":2, "class":"b", "opcode":12, "category":"interrupt", "signed":false, "description":"IAQ a : If a is nonzero, interrupts will be added to the queue instead of triggered. If a is 0, interrupts will be triggered as normal again"},
{"name":"HWN", "cycle":2, "class":"b", "opcode":16, "category":"hardware", "signed":false, "description":"HWN a : Sets a to the number of connected hardware devices"},
{"name":"HWQ", "cycle":4, "class":"b", "opcode":17, "category":"hardware", "signed":false, "description":"HWQ a : Sets A, B, C, X, Y registers to information about hardware a. A+(B<<16) is a 32 bit word identifying the hardware id. C is hardware version. X+(Y<<16) is a 32bit word identifying the manufacturer"},
{"name":"HWI", "cycle":4, "class":"b", "opcode":18, "category":"hardware", "signed":false, "description":"HWI a : Sends an interrupt to hardware a"},
{"name":"IFW", "cycle":3, "class":"b", "opcode":26, "category":"control", "signed":false, "description":"IFW a : Performs next instruction only if the channel identified by a has any value waiting to be written"},
{"name":"IFR", "cycle":3, "class":"b", "opcode":27, "category":"control", "signed":false, "description":"IFR a : Performs next instruction only if the channel identified by a has any value waiting to be written"},
{"name":"BRK", "cycle":1, "class":"c", "opcode":0, "category":"control", "signed":false, "description":"BRK : Terminates execution"}
]
)end"
    );

    return data;
}

static std::string format_hex(uint16_t val, bool is_sign)
{
    std::stringstream str;

    if(!is_sign)
        str << std::hex << val;
    else
        str << std::hex << (int16_t)val;

    std::string rval = str.str();

    for(int i=(int)rval.size(); i < 4; i++)
    {
        rval = "0" + rval;
    }

    return "0x" + rval;
}

///formatted to same width as format_hex
static std::string format_dec(uint16_t val, bool is_sign)
{
    std::string out;

    if(!is_sign)
        out = std::to_string(val);
    else
        out = std::to_string((int16_t)val);

    for(int i=(int)out.size(); i < 6; i++)
    {
        out = " " + out;
    }

    return out;
}

static std::string format_hex_or_dec(uint16_t val, bool hex, bool is_sign)
{
    return hex ? format_hex(val, is_sign) : format_dec(val, is_sign);
}

void register_editor(const std::string& name, uint16_t& val, bool is_hex, bool is_sign, bool is_modifiable)
{
    ImGui::Text(name.c_str());

    ImGui::SameLine();

    ImGui::PushItemWidth(100);

    int step = 1;

    if(is_modifiable)
    {
        int32_t ival = (is_sign && !is_hex) ? (int32_t)(int16_t)val : (int32_t)val;

        if(!is_hex)
            ImGui::InputInt(("##" + name).c_str(), &ival);
        else
            ImGui::InputScalar(("##" + name).c_str(), ImGuiDataType_S32, (void*)&ival, (void*)&step, nullptr, "%04X", 0);

        val = ival;
    }
    else
    {
        ImGui::Text(("| " + format_hex_or_dec(val, is_hex, is_sign)).c_str());
    }
}

void dcpu::ide::settings::load(const std::string& file)
{
    toml::value val = toml::parse(file);

    files = toml::get<std::vector<std::string>>(val["files"]);
}

dcpu::ide::editor::editor()
{
    edit = new TextEditor;
    auto lang = TextEditor::LanguageDefinition::CPlusPlus();
    lang.mCaseSensitive = false;
    lang.mSingleLineComment = ";";
    lang.mAutoIndentation = false;

    lang.mKeywords.clear();
    lang.mIdentifiers.clear();
    lang.mPreprocIdentifiers.clear();

    auto mapping = instruction_to_description();

    for(auto& i : mapping)
    {
        TextEditor::Identifier id;
        id.mDeclaration = i["description"];
        lang.mIdentifiers.insert(std::make_pair(i["name"], id));
    }

    edit->SetLanguageDefinition(lang);

    auto palette = edit->GetPalette();

    palette[(int)TextEditor::PaletteIndex::Breakpoint] = IM_COL32(150, 40, 40, 128);

    edit->SetPalette(palette);
}

void dcpu::ide::editor::render_inline(project_instance& instance, int id)
{
    std::string cycle_string = "Cycles: " + std::to_string(c.cycle_count);

    bool popup = false;

    if(ImGui::BeginMenuBar())
    {
        if(ImGui::MenuItem("Assemble"))
        {
            wants_assemble = true;
        }

        if(ImGui::MenuItem("Step"))
        {
            wants_step = true;
        }

        if(!is_running)
        {
            if(ImGui::MenuItem("Run"))
            {
                wants_run = true;
            }
        }
        else
        {
            if(ImGui::MenuItem("Pause"))
            {
                wants_pause = true;
            }
        }

        if(ImGui::MenuItem("Reset"))
        {
            wants_reset = true;
        }

        if(ImGui::BeginMenu("Settings"))
        {
            std::string is_hex_str = is_hex ? "[x] Hex###hexid" : "[ ] Hex###hexid";

            is_hex_str += std::to_string(id);

            if(ImGui::MenuItem(is_hex_str.c_str()))
            {
                is_hex = !is_hex;
            }

            std::string is_signed_str = is_sign ? "[x] Signed###signedid" : "[ ] Signed###signedid";

            is_signed_str += std::to_string(id);

            if(ImGui::MenuItem(is_signed_str.c_str()))
            {
                is_sign = !is_sign;
            }

            std::string is_modifiable_str = is_modifiable ? "[x] Reg-Editor###regid" : "[ ] Reg-Editor###regid";

            is_modifiable_str += std::to_string(id);

            if(ImGui::MenuItem(is_modifiable_str.c_str()))
            {
                is_modifiable = !is_modifiable;
            }

            std::string is_autosave_str = instance.is_autosaving ? "[x] Autosave###autosaveid" : "[ ] Autosave###autosaveid";

            is_autosave_str += std::to_string(id);

            if(ImGui::MenuItem(is_autosave_str.c_str()))
            {
                instance.is_autosaving = !instance.is_autosaving;
            }

            if(ImGui::MenuItem("Frequency-Editor"))
            {
                popup = true;
            }

            ImGui::EndMenu();
        }

        ImGui::MenuItem((cycle_string + "###cyclid" + std::to_string(id)).c_str());

        ImGui::EndMenuBar();
    }

    if(popup)
    {
        ImGui::OpenPopup("Frequency-Editor");
    }

    if(ImGui::BeginPopup("Frequency-Editor"))
    {
        int old = clock_hz;

        ImGui::SliderInt("Ticks Per Second", &clock_hz, 1, 1000);

        if(clock_hz < 1)
            clock_hz = 1;

        if(clock_hz > 1000)
            clock_hz = 1000;

        dirty_frequency = old != clock_hz;

        if(ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Ctrl+Left click to edit");
        }

        ImGui::EndPopup();
    }

    if(ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
    {
        if(ImGui::IsKeyDown(GLFW_KEY_LEFT_CONTROL) && ImGui::IsKeyPressed(GLFW_KEY_S, false))
        {
            instance.save();
        }
    }

    ImGui::BeginGroup();

    register_editor("A ", c.regs[A_REG], is_hex, is_sign, is_modifiable);
    register_editor("B ", c.regs[B_REG], is_hex, is_sign, is_modifiable);
    register_editor("C ", c.regs[C_REG], is_hex, is_sign, is_modifiable);
    register_editor("X ", c.regs[X_REG], is_hex, is_sign, is_modifiable);
    register_editor("Y ", c.regs[Y_REG], is_hex, is_sign, is_modifiable);
    register_editor("Z ", c.regs[Z_REG], is_hex, is_sign, is_modifiable);
    register_editor("I ", c.regs[I_REG], is_hex, is_sign, is_modifiable);
    register_editor("J ", c.regs[J_REG], is_hex, is_sign, is_modifiable);
    register_editor("PC", c.regs[PC_REG], is_hex, is_sign, is_modifiable);
    register_editor("SP", c.regs[SP_REG], is_hex, is_sign, is_modifiable);
    register_editor("EX", c.regs[EX_REG], is_hex, is_sign, is_modifiable);
    register_editor("IA", c.regs[IA_REG], is_hex, is_sign, is_modifiable);

    if(error_string.size() > 0)
    {
        std::map<int, std::string> error_marker;
        error_marker[error_line + 1] = error_string;

        if(dirty_errors)
            edit->SetErrorMarkers(error_marker);

        /*if(ImGui::Button("Clear Errors"))
        {
            error_marker.clear();
            edit->SetErrorMarkers(error_marker);
            error_string.clear();
        }*/
    }
    else
    {
        std::map<int, std::string> error_marker;
        edit->SetErrorMarkers(error_marker);
    }

    dirty_errors = false;

    ImGui::EndGroup();

    ImGui::SameLine();

    int ysize = ImGui::GetContentRegionAvail().y;

    std::string full_error_str = "Error:\n" + error_string;

    int error_height = ImGui::CalcTextSize(full_error_str.c_str()).y;

    int offset = 10;

    if(error_string.size() == 0)
    {
        offset = 0;
        error_height = 0;
    }

    ImGui::BeginChild(("Child" + std::to_string(id)).c_str(), ImVec2(0, ysize - error_height - offset));

    {
        //std::unordered_set<int> current_pc;

        std::map<int, ImVec4> current_pc;

        if(c.regs[PC_REG] < translation_map.size())
        {
            int line = pc_to_source_line[c.regs[PC_REG]];

            current_pc[line + 1] = ImVec4(0, 0x80/255.f, 0xf0/255.f, 0x40/255.f);
        }

        edit->SetHighlightLines(current_pc);
    }

    edit->Render((std::string("IDEW") + std::to_string(id)).c_str());

    if(edit->IsTextChanged())
        unsaved = true;

    ImGui::EndChild();

    if(error_height > 0)
    {
        ImGui::BeginChild("IDECHILD", ImVec2(0, error_height));

        if(error_string.size() > 0)
        {
            ImGui::Text("%s\n", full_error_str.c_str());
        }

        ImGui::EndChild();
    }

    if(instance.is_autosaving && unsaved)
    {
        std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();

        double result = std::chrono::duration<double>(now - instance.autosave_timer).count();

        if(result > 5)
        {
            instance.autosave_timer = now;
            instance.save();
        }
    }

    ///breakpoint handling
    {
        std::unordered_set<int> found_breakpoints = edit->GetBreakpoints();

        stack_vector<uint16_t, 256> linear_program_counters;

        for(auto& i : found_breakpoints)
        {
            linear_program_counters.push_back(source_line_to_pc[(uint16_t)(i - 1)]);

            if(linear_program_counters.size() >= linear_program_counters.max_size)
                break;
        }

        c.set_breakpoints(linear_program_counters);
    }
}

void dcpu::ide::editor::render(project_instance& instance, int id)
{
    std::string root_name = "IDE";

    if(unsaved)
        root_name += " (unsaved)";

    ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_Appearing);

    ImGui::Begin((root_name + "###IDE" + std::to_string(id)).c_str(), nullptr, ImGuiWindowFlags_MenuBar);

    render_inline(instance, id);

    ImGui::End();
}

void dcpu::ide::editor::handle_default_step()
{
    if(wants_step)
    {
        wants_step = false;
        wants_run = false;
        is_running = false;

        if(!halted)
            halted = halted || c.step();
    }

    if(wants_assemble || wants_reset)
    {
        wants_reset = false;
        wants_assemble = false;
        wants_run = false;
        assemble();

        is_running = false;
        halted = false;
    }

    if(wants_run)
    {
        wants_run = false;
        is_running = true;
        halted = false;
    }

    if(wants_pause)
    {
        wants_pause = false;
        is_running = false;
    }

    if(is_running)
    {
        ///bad
        if(!halted)
            halted = halted || c.step();
    }
}

bool dcpu::ide::editor::assemble()
{
    auto [rinfo_opt, err] = assemble_fwd(get_text());

    dirty_errors = true;

    if(rinfo_opt.has_value())
    {
        c = dcpu::sim::CPU();
        c.load(rinfo_opt.value().mem, 0);
        halted = false;

        translation_map = rinfo_opt.value().translation_map;
        pc_to_source_line = rinfo_opt.value().pc_to_source_line;
        source_line_to_pc = rinfo_opt.value().source_line_to_pc;

        error_string.clear();
        error_line = 0;

        return false;
    }
    else
    {
        std::string formatted = format_error(err);

        error_string = formatted;
        error_line = err.line;

        return true;
    }
}

std::string dcpu::ide::editor::get_text() const
{
    std::string str = edit->GetText();

    if(str.size() > 0 && str.back() == '\n')
        str.pop_back();

    return str;
}

void dcpu::ide::editor::set_text(const std::string& str)
{
    edit->SetText(str);
}

void dcpu::ide::reference_card::render()
{
    ImGui::Begin("Reference", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    render_inline();

    ImGui::End();
}

void dcpu::ide::reference_card::render_inline()
{
    auto all_instr = instruction_to_description();

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
                    ImGui::BeginTooltip();

                    ImGui::PushTextWrapPos(400);

                    ImGui::Text("%s", desc.c_str());

                    ImGui::EndTooltip();
                }
            }

            ImGui::TreePop();
        }
    }
}
