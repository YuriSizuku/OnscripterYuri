#include <stdio.h>
#include <emscripten.h>

void mainloop()
{

}

int main(int argc, char **argv)
{
    emscripten_set_main_loop(mainloop, -1, -1);
    return 0;
}