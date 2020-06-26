#ifndef BASE_IDE_HPP_INCLUDED
#define BASE_IDE_HPP_INCLUDED

#include <imguicolortextedit/TextEditor.h>

namespace dcpu
{
    namespace instruction
    {
        enum category
        {
            NONE = 0,
            ARITHMETIC = 1,
            CONTROLFLOW = 2,
            SIGNED = 4,
            UNSIGNED = 8,
            LOGICAL = 16,
            INTERRUPT = 32,
            MULTIPROCESSOR = 64,
            MISC = 64,
        };

        struct metadata
        {
            std::string desc;
            category cat = instruction::NONE;
        };
    }

    inline
    const std::vector<std::pair<std::string, instruction::metadata>>& instruction_to_description()
    {
        static std::vector<std::pair<std::string, instruction::metadata>> instr
        {
             {"SET", "SET b, a : Sets b to a"},
             {"ADD", "ADD b, a : Sets b to b+a, sets EX to 0x0001 on overflow"},
             {"SUB","SUB b, a : Sets b to b-a, sets EX to 0xffff on underflow"},
             {"MUL","MUL b, a : Sets b to b * a, sets EX to ((b*a)>>16)&0xffff. (b, a) are unsigned"},
             {"MLI","MLI b, a : Like MUL. (b, a) are signed"},
             {"DIV","DIV b, a : Sets b to b/a, sets EX to ((b<<16)/a)&0xffff. If a == 0, sets b and EX to 0. (b, a) are unsigned"},
             {"DVI","DVI b, a : Like DIV. (b, a) are unsigned. Rounds towards 0"},
             {"MOD","MOD b, a : Sets b to b%a. If a==0, sets b to 0 instead"},
             {"MDI","MDI b, a : Like MOD. (b, a) are signed"},
             {"AND","AND b, a : Sets b to b&a"},
             {"BOR","BOR b, a : Sets b to b|a"},
             {"XOR","XOR b, a : Sets b to b^a"},
             {"SHR","SHR b, a : Sets b to b>>>a, sets EX to ((b<<16)>>a)&0xffff. Logical shift"},
             {"ASR","ASR b, a : Sets b to b>>a, sets EX to ((b<<16)>>>a)&0xffff. Arithmetic shift. (b) is signed"},
             {"SHL","SHL b, a : Sets b to b<<a, sets EX to ((b<<a)>>16)&0xffff"},
             {"IFB","IFB b, a : Performs next instruction only if (b&a)!=0"},
             {"IFC","IFC b, a : Performs next instruction only if (b&a)==0"},
             {"IFE","IFE b, a : Performs next instruction only if b==a"},
             {"IFN","IFN b, a : Performs next instruction only if b!=a"},
             {"IFG","IFG b, a : Performs next instruction only if b>a"},
             {"IFA","IFA b, a : Performs next instruction only if b>a (signed)"},
             {"IFL","IFL b, a : Performs next instruction only if b<a"},
             {"IFU","IFU b, a : Performs next instruction only if b<a (signed)"},
             {"ADX","ADX b, a : Sets b to b+a+EX, sets EX to 0x0001 on overflow, otherwise 0x0"},
             {"SBX","SBX b, a : Sets b to b-a+EX, sets EX to 0xffff on overflow, otherwise 0x0"},
             {"SND","SND b, a : Sends the value b to the hardware with identifier a, blocks until sent"},
             {"RCV","RCV b, a : Sets b to a value received from the hardware with identifier a, blocks until received"},
             {"STI","STI b, a : Sets b to a, then increases I and J by 1"},
             {"STD","STD b, a : Sets b to a, then decreases I and J by 1"},
             {"JSR","JSR a : Pushes the address of the next instruction to the stack, then sets PC to a"},
             {"INT","INT a : Triggers a software interrupt with message a"},
             {"IAG","IAG a : Sets a to IA"},
             {"IAS","IAS a : Sets IA to a"},
             {"RFI","RFI a : Disables interrupt queueing, pops A from the stack, then pops PC from the stack"},
             {"IAQ","IAQ a : If a is nonzero, interrupts will be added to the queue instead of triggered. If a is 0, interrupts will be triggered as normal again"},
             {"HWN","HWN a : Sets a to the number of connected hardware devices"},
             {"HWQ","HSQ a : Sets A, B, C, X, Y registers to information about hardware a. A+(B<<16) is a 32 bit word identifying the hardware id. C is hardware version. X+(Y<<16) is a 32bit word identifying the manufacturer"},
             {"HWI","HWI a : Sends an interrupt to hardware a"},
             {"IFW","IFW a : Performs next instruction only if the hardware identified by a is waiting to write a value"},
             {"IFR","IFR a : Performs next instruction only if the hardware identified by a is waiting to read a value"},
             {"BRK","BRK : Pauses execution"},
        };

        return instr;
    }

    inline
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
    }

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
                    id.mDeclaration = i.second;
                    lang.mIdentifiers.insert(std::make_pair(i.first, id));
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
