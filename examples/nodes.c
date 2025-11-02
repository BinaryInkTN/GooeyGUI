#include "gooey.h"

// Node editor callback
void node_editor_callback(void* user_data)
{
    printf("Node editor updated!\n");

}

int main()
{
    Gooey_Init();

    GooeyWindow *win = GooeyWindow_Create("Node Editor Example", 800, 600, true);
    GooeyWindow_MakeResizable(win, true);
    GooeyNodeEditor* node_editor = GooeyNodeEditor_Create(50, 50, 700, 500, node_editor_callback, NULL);
    GooeyNodeEditor_AddNode(node_editor, "Input Node", 100, 100, 150, 100);
    GooeyNodeEditor_AddNode(node_editor, "Input Node", 100, 100, 150, 100);
    GooeyNodeEditor_AddNode(node_editor, "Output Node", 400, 300, 150, 100);
    GooeyNodeSocket *output_socket = GooeyNode_AddSocket(node_editor->nodes[0], "Output", GOOEY_SOCKET_TYPE_OUTPUT,
                                                         GOOEY_DATA_TYPE_FLOAT);
    GooeyNodeSocket* input_socket = GooeyNode_AddSocket(node_editor->nodes[1], "Input", GOOEY_SOCKET_TYPE_INPUT, GOOEY_DATA_TYPE_FLOAT);
    GooeyNodeSocket* input_socket_2 = GooeyNode_AddSocket(node_editor->nodes[2], "Input", GOOEY_SOCKET_TYPE_INPUT, GOOEY_DATA_TYPE_FLOAT);
    GooeyNodeEditor_ConnectSockets(node_editor, output_socket, input_socket);

    GooeyWindow_RegisterWidget(win, node_editor);
    GooeyWindow_Run(1, win);
    GooeyWindow_Cleanup(1, win);

    return 0;
}