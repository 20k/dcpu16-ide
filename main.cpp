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
