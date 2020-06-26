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

int main()
{
    render_settings sett;
    sett.width = 800;
    sett.height = 600;

    render_window win(sett, "DCPU16-IDE");

    TextEditor edit;

    auto lang = TextEditor::LanguageDefinition::CPlusPlus();

    static const char* identifiers[] = {"SET",
                                         "ADD",
                                         "SUB",
                                         "MUL",
                                         "MLI",
                                         "DIV",
                                         "DVI",
                                         "MOD",
                                         "MDI",
                                         "AND",
                                         "BOR",
                                         "XOR",
                                         "XHR",
                                         "ASR",
                                         "SHL",
                                         "IFB",
                                         "IFC",
                                         "IFE",
                                         "IFN",
                                         "IFG",
                                         "IFA",
                                         "IFL",
                                         "IFU",
                                         "ADX",
                                         "SBX",
                                         "SND",
                                         "RCV",
                                         "STI",
                                         "STD",
                                         "JSR",
                                         "INT",
                                         "IAG",
                                         "IAS",
                                         "RFI",
                                         "IAQ",
                                         "HWN",
                                         "HWQ",
                                         "HWI",
                                         "IFW",
                                         "IFR",
                                         "BRK"};

    static const char* descriptions[] = {
        "SET b, a : Sets b to a",
        "ADD b, a : Sets b to b+a, sets EX to 0x0001 on overflow",
        "SUB b, a : Sets b to b-a, sets EX to 0xffff on underflow",
        "MUL b, a : Sets b to b * a, sets EX to ((b*a)>>16)&0xffff. (b, a) are unsigned",
        "MLI b, a : Like MUL. (b, a) are signed",
        "DIV b, a : Sets b to b/a, sets EX to ((b<<16)/a)&0xffff. If a == 0, sets b and EX to 0. (b, a) are unsigned",
        "DVI b, a : Like DIV. (b, a) are unsigned. Rounds towards 0",
        "MOD b, a : Sets b to b%a. If a==0, sets b to 0 instead",
        "MDI b, a : Like MOD. (b, a) are signed",
        "AND b, a : Sets b to b&a",
        "BOR b, a : Sets b to b|a",
        "XOR b, a : Sets b to b^a",
        "SHR b, a : Sets b to b>>>a, sets EX to ((b<<16)>>a)&0xffff. Logical shift",
        "ASR b, a : Sets b to b>>a, sets EX to ((b<<16)>>>a)&0xffff. Arithmetic shift. (b) is signed",
        "SHL b, a : Sets b to b<<a, sets EX to ((b<<a)>>16)&0xffff",
        "IFB b, a : Performs next instruction only if (b&a)!=0",
        "IFC b, a : Performs next instruction only if (b&a)==0",
        "IFE b, a : Performs next instruction only if b==a",
        "IFN b, a : Performs next instruction only if b!=a",
        "IFG b, a : Performs next instruction only if b>a",
        "IFA b, a : Performs next instruction only if b>a (signed)",
        "IFL b, a : Performs next instruction only if b<a",
        "IFU b, a : Performs next instruction only if b<a (signed)",
        "ADX b, a : Sets b to b+a+EX, sets EX to 0x0001 on overflow, otherwise 0x0",
        "SBX b, a : Sets b to b-a+EX, sets EX to 0xffff on overflow, otherwise 0x0",
        "SND b, a : Sends the value b to the hardware with identifier a, blocks until sent",
        "RCV b, a : Sets b to a value received from the hardware with identifier a, blocks until received",
        "STI b, a : Sets b to a, then increases I and J by 1",
        "STD b, a : Sets b to a, then decreases I and J by 1",
        "JSR a : Pushes the address of the next instruction to the stack, then sets PC to a",
        "INT a : Triggers a software interrupt with message a",
        "IAG a : Sets a to IA",
        "IAS a : Sets IA to a",
        "TFI a : Disables interrupt queueing, pops A from the stack, then pops PC from the stack",
        "IAQ a : If a is nonzero, interrupts will be added to the queue instead of triggered. If a is 0, interrupts will be triggered as normal again",
        "HWN a : Sets a to the number of connected hardware devices",
        "HSQ a : Sets A, B, C, X, Y registers to information about hardware a. A+(B<<16) is a 32 bit word identifying the hardware id. C is hardware version. X+(Y<<16) is a 32bit word identifying the manufacturer",
        "HWI a : Sends an interrupt to hardware a",
        "IFW a : Performs next instruction only if the hardware identified by a is waiting to write a value",
        "IFR a : Performs next instruction only if the hardware identified by a is waiting to read a value",
        "BRK : Pauses execution",
    };

    /*for (int i = 0; i < sizeof(identifiers) / sizeof(identifiers[0]); ++i)
	{
		TextEditor::Identifier id;
		id.mDeclaration = descriptions[i];
		lang.mPreprocIdentifiers.insert(std::make_pair(std::string(identifiers[i]), id));
	}*/

	for (int i = 0; i < sizeof(identifiers) / sizeof(identifiers[0]); ++i)
	{
		TextEditor::Identifier id;
		id.mDeclaration = std::string(descriptions[i]);
		lang.mIdentifiers.insert(std::make_pair(std::string(identifiers[i]), id));
	}

	edit.SetLanguageDefinition(lang);

    while(!win.should_close())
    {
        win.poll();

        ImGui::Begin("Hello");

        edit.Render("Hithere");

        ImGui::End();

        win.display();
    }

    return 0;
}
