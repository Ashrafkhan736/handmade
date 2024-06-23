#include <windows.h>
#include <stdio.h>

int CALLBACK WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nShowCmd)

{
    MessageBoxA(0, "This is a handmade hero", "Handmade hero", MB_OK | MB_ICONINFORMATION);
    return 0;
};

// int main()
// {
//     printf("hello world");

//     return 0;
// }