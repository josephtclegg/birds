// birds.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "birds.h"
#include "Shader.h"
#include "camera.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <windows.h>
#include <CommCtrl.h>
#include <scrnsave.h>
#include <GL/glew.h>
#include <GL/glut.h>
//#include <GL/wgl.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//#include <GL/GL.h>

#define MAX_LOADSTRING 100
#define IDS_DESCRIPTION 1 //description in menu
#define TIMER 1                         //define a Windows timer
extern TCHAR szAppName[APPNAMEBUFFERLEN] = TEXT("lucy screensaver");          //name in menu

//get rid of these warnings:
//truncation from const double to float
//conversion from double to float
#pragma warning(disable: 4305 4244) 

//These forward declarations are just for readability,
//so the big three functions can come first 

void InitGL(HWND hWnd, HDC& hDC, HGLRC& hRC);
void CloseGL(HWND hWnd, HDC hDC, HGLRC hRC);
void SetupAnimation(int Width, int Height);
void CleanupAnimation();
void OnTimer(HDC hDC);
GLint compileShader(const char* filename, GLenum type);
GLint compileShaderProgram(const char* vertexShaderFilename, const char* fragmentShaderFilename);
GLint loadTexture(const char* texture);
SYSTEMTIME st;

int Width, Height; //globals for size of screen

//GLuint ourShader;
Shader *ourShader;
GLuint lucy, lucy_reflect;
GLuint vbo, vao, ebo, lvao;
GLfloat xvel = 0.0007;
GLfloat yvel = 0.0017;
GLfloat xpos = 0.0;
GLfloat ypos = 0.0;
Camera camera(glm::vec3(0.0f, 0.0f, 50.0f));
glm::vec3 lightPos(0.0f, 0.0f, 100.0f);
unsigned char DATA[999999];

void GetPngFromResource(int iResourceID, void** ppPngFileData, DWORD* pwPngDataBytes)
{
    HMODULE hMod = GetModuleHandle(NULL);
    HRSRC hRes = FindResource(hMod, MAKEINTRESOURCE(iResourceID), _T("PNG"));
    HGLOBAL hGlobal = LoadResource(hMod, hRes);
    *ppPngFileData = LockResource(hGlobal);
    *pwPngDataBytes = SizeofResource(hMod, hRes);
}

GLuint LoadTexture(GLuint tex, int rc)
{
    HANDLE hBitmap;
    BITMAP bm;
    HINSTANCE hInstance = GetModuleHandle(NULL);

    //standard bmp 24 bit
    //supported resolutions 64x64, 128x128, 256x256, 512x512

    //type "char" has a 1 byte size, other types take more byte and will not work
    unsigned char* data;
    unsigned char R, G, B;

    //LoadImage() loads the bmp picture as an interlaced image
    //hBitmap = LoadImage(NULL, filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    hBitmap = LoadImage(hInstance, MAKEINTRESOURCE(rc), IMAGE_BITMAP, 0, 0, NULL);

    GetObject(hBitmap, sizeof(BITMAP), &bm);

    //get the address of the start of the image data in memory 
    data = (unsigned char *)bm.bmBits;

    //swap R, G, B values for correct color display

    int index, i;

    for (i = 0; i < bm.bmWidth * bm.bmHeight; i++)
    {
        index = i * 3;
        B = data[index];
        G = data[index + 1];
        R = data[index + 2];
        data[index] = R;
        data[index + 1] = G;
        data[index + 2] = B;
    }

    //print image parameters
    printf("bmType %u\n", bm.bmType);
    printf("bmWidth %u\n", bm.bmWidth);
    printf("bmHeight %u\n", bm.bmHeight);
    printf("bmWidthBytes %u\n", bm.bmWidthBytes);
    printf("bmPlanes %u\n", bm.bmPlanes);
    printf("bmBitsPixel %u\n", bm.bmBitsPixel);
    printf("bmBits %p\n", bm.bmBits);
    printf("hInstance %p\n", hInstance);

    //create texture from loaded bmp image 
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, 4, bm.bmWidth, bm.bmHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, bm.bmBits);

    printf("--- texture %u created ---\n", tex);

    //texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    return tex;
}

// Screen Saver Procedure
LRESULT WINAPI ScreenSaverProc(HWND hWnd, UINT message,
    WPARAM wParam, LPARAM lParam)
{
    static HDC hDC;
    static HGLRC hRC;
    static RECT rect;

    switch (message) {

    case WM_CREATE:
        // get window dimensions
        GetClientRect(hWnd, &rect);
        Width = rect.right;
        Height = rect.bottom;

        // setup OpenGL, then animation
        InitGL(hWnd, hDC, hRC);
        SetupAnimation(Width, Height);

        //set timer to tick every 10 ms
        SetTimer(hWnd, TIMER, 10, NULL);
        return 0;

    case WM_DESTROY:
        KillTimer(hWnd, TIMER);
        CleanupAnimation();
        CloseGL(hWnd, hDC, hRC);
        return 0;

    case WM_TIMER:
        OnTimer(hDC);       //animate!      
        return 0;

    }

    return DefScreenSaverProc(
        hWnd, message, wParam, lParam);

}

bool bTumble = true;


BOOL WINAPI
ScreenSaverConfigureDialog(HWND hDlg, UINT message,
    WPARAM wParam, LPARAM lParam)
{

    //InitCommonControls();  
    //would need this for slider bars or other common controls

    HWND aCheck;

    switch (message)
    {

    case WM_INITDIALOG:
        LoadString(hMainInstance, IDS_DESCRIPTION, szAppName, 40);

        return TRUE;

    }     //end command switch

    return FALSE;
}



// needed for SCRNSAVE.LIB
BOOL WINAPI RegisterDialogClasses(HANDLE hInst)
{
    return TRUE;
}

// Initialize OpenGL
static void InitGL(HWND hWnd, HDC& hDC, HGLRC& hRC)
{
    PIXELFORMATDESCRIPTOR pfd;
    ZeroMemory(&pfd, sizeof pfd);
    pfd.nSize = sizeof pfd;
    pfd.nVersion = 1;
    //pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL; //blaine's
    pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;

    hDC = GetDC(hWnd);

    int i = ChoosePixelFormat(hDC, &pfd);
    SetPixelFormat(hDC, i, &pfd);

    hRC = wglCreateContext(hDC);
    wglMakeCurrent(hDC, hRC);

    camera.Zoom = 50.0f;

    glewExperimental = GL_TRUE;

    if (glewInit() != GLEW_OK) {
        //abort();
        std::cout << "awg god man glewInit failed" << std::endl;
        abort();
        return;
    }

 /*   const GLint program = glCreateProgram();

    if (not program) {
        return;
    }
    */

    //window resizing stuff
    glViewport(0, 0, (GLsizei)Width, (GLsizei)Height);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    ourShader = new Shader("vertex.vs", "fragment.vs");

    /*
    const GLint shader = compileShaderProgram("vertex.vs", "fragment.vs");

    PIXELFORMATDESCRIPTOR pfd;
    ZeroMemory(&pfd, sizeof pfd);
    pfd.nSize = sizeof pfd;
    pfd.nVersion = 1;
    //pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL; //blaine's
    pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;

    hDC = GetDC(hWnd);

    int i = ChoosePixelFormat(hDC, &pfd);
    SetPixelFormat(hDC, i, &pfd);

    hRC = wglCreateContext(hDC);
    wglMakeCurrent(hDC, hRC);

    glGenTextures(1, &lucy);
    glBindTexture(GL_TEXTURE_2D, lucy);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load("lucyfont_filled.png", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    else {
        std::cout << "awhg gof i cant find the texture!!!!!:<:<" << std::endl;
    }

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, lucy);

    glUniform1f(glGetUniformLocation(shader, "texture0"), 0);
    */

}

// Shut down OpenGL
static void CloseGL(HWND hWnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);

    ReleaseDC(hWnd, hDC);
}

void SetupAnimation(int Width, int Height)
{

    GLfloat vertices[] = {
        // Positions          // Normals           // Texture Coords
       0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // Top Right
       0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f, // Bottom Right
       -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // Bottom Left
       -0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f  // Top Left   
    };
    GLuint indices[] =
    {  // Note that we start from 0!
        0, 1, 3, // First Triangle
        1, 2, 3  // Second Triangle
    };
    GLuint VAO, VBO, EBO;

    if (glGenVertexArrays == NULL) {
        std::cout << "omg glGenVertexArrays nulL!!!!!" << std::endl;
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &lvao);

    vao = VAO;
    vbo = VBO;
    ebo = EBO;

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    // Texture Coordinate attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(lvao);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    //glBindVertexArray(0); // Unbind VAO

    // Load and create a texture
    //GLuint texture;

    //int width, height;


    // ===================
    // Texture
    // ===================
   
    glGenTextures(1, &lucy);
    glBindTexture(GL_TEXTURE_2D, lucy);
    // Set our texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // Set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    //unsigned char *data;
    //DWORD data_size;
    //GetPngFromResource(131, (void**) & DATA, &data_size);
    //lucy = LoadTexture(lucy, IDB_PNG1);
    unsigned char *data = stbi_load("lucyfont_filled.png", &width, &height, &nrChannels, STBI_rgb_alpha);
    //unsigned char* data =  (unsigned char *) lucyfont_filled;
    //width = LUCYFONT_FILLED_WIDTH;
    //height = LUCYFONT_FILLED_HEIGHT;
    //nrChannels = 2;

    if (data) {
        //std::ofstream file;
        //file.open("C:/Users/nxie/Documents/lucy_binary.txt", std::ios_base::binary | std::ios_base::out);
        //assert(file.is_open());
        //file.write((const char*)data, (width*height)/sizeof(unsigned char));
        //file.close();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        //glBindTexture(GL_TEXTURE_2D, 0);
    }
    else {

        std::cout << "awhg gof i cant find the texture!!!!!:<:<" << std::endl;
    }
    
    stbi_image_free(data);



    glGenTextures(1, &lucy_reflect);
    glBindTexture(GL_TEXTURE_2D, lucy_reflect);
    // Set our texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // Set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    data = stbi_load("lucyfont_reflect.png", &width, &height, &nrChannels, STBI_rgb_alpha);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        //glBindTexture(GL_TEXTURE_2D, 0);
    }
    else {

        std::cout << "awhg gof i cant find the other texture!!!!!:<:<" << std::endl;
    }
    stbi_image_free(data);

    glMatrixMode(GL_MODELVIEW);
    //glLoadIdentity();

    //glOrtho(-300, 300, -240, 240, 25, 75);
    //glMatrixMode(GL_MODELVIEW);

    //glLoadIdentity();
    //gluLookAt(0.0, 0.0, 50.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glShadeModel(GL_SMOOTH);
    //camera xyz, the xyz to look at, and the up vector (+y is up)

/*    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(-300, 300, -240, 240, 25, 75);
    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();
    gluLookAt(0.0, 0.0, 50.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    //camera xyz, the xyz to look at, and the up vector (+y is up)

//background
    glClearColor(0.0, 0.0, 0.0, 0.0); //0.0s is black


    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glShadeModel(GL_SMOOTH);

    //no need to initialize any objects
    //but this is where I'd do it

    glColor3f(0.1, 1.0, 0.3); //green
    */


}

static GLfloat spin = 0;   //a global to keep track of the square's spinning

void OnTimer(HDC hDC) //increment and display
{
    GetSystemTime(&st);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (xpos > 1.0) {
        xvel = xvel * -1.0;
    }
    if (xpos < 0.0) {
        xvel = xvel * -1.0;
    }
    if (ypos > 0.5) {
        yvel = yvel * -1.0;
    }
    if (ypos < -0.5) {
        yvel = yvel * -1.0;
    }
    xpos += xvel;
    ypos += yvel;

    glm::mat4 transform = glm::mat4(1.0f); //identity
    transform = glm::translate(transform, glm::vec3(xpos, ypos, 0.0f));
    //transform = glm::translate(transform, glm::vec3(0.5f, -0.5f, 0.0f));
    transform = glm::rotate(transform, (spin/35.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    transform = glm::scale(transform, glm::vec3(0.33, 0.33, 0.33));
    //transform = glm::translate(transform, glm::vec3(-0.5f, 0.5f, 0.0f));

    ourShader->Use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, lucy);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, lucy_reflect);
    glUniform1i(glGetUniformLocation(ourShader->Program, "texture0"), 0);
    glUniform1i(glGetUniformLocation(ourShader->Program, "texture1"), 1);
    glUniform1f(glGetUniformLocation(ourShader->Program, "st"), spin);
    glUniform2f(glGetUniformLocation(ourShader->Program, "iResolution"), (GLfloat)Width, (GLfloat)Height);
    glUniformMatrix4fv(glGetUniformLocation(ourShader->Program, "transform"), 1, GL_FALSE, glm::value_ptr(transform));
    glUniform3f(glGetUniformLocation(ourShader->Program, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
    glUniform3f(glGetUniformLocation(ourShader->Program, "viewPos"), 0.0, 0.0, 0.5);

    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)Width / (float)Height, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(ourShader->Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(ourShader->Program, "view"), 1, GL_FALSE, glm::value_ptr(projection));

    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(ourShader->Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

    
    spin = spin + 1;

    //glPushMatrix();
    /*
    glRotatef(spin, 0.0, 0.0, 1.0);

    glPushMatrix();

    glTranslatef(150, 0, 0);

    if (bTumble)
        glRotatef(spin * -3.0, 0.0, 0.0, 1.0);
    else
        glRotatef(spin * -1.0, 0.0, 0.0, 1.0);
        */

    //glMatrixMode(GL_PROJECTION);

  
    //glPushMatrix();
    //glLoadIdentity();
    //glRotatef(spin * -3.0, 0.0, 0.0, 1.0); // 2. Rotate the object.

    //glTranslatef(-250, -250, 0.0); // 1. Translate to the origin.

    // Draw the object
    //glPopMatrix();

    //glRotatef(spin * -30.0, 0.0, 1.0, 0.0); // 2. Rotate the object.

    //glBegin(GL_TRIANGLES);


    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    //glTranslatef(20, 20, 0.0); // 3. Translate to the object's position.
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 27, GL_UNSIGNED_INT, 0);
    //glBindVertexArray(lvao);
    //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    //glPopMatrix();

    //draw the square (rotated to be a diamond)

    /*
    float xvals[] = { -30.0, 0.0, 30.0, 0.0 };
    float yvals[] = { 0.0, -30.0, 0.0, 30.0 };
    float xtexvals[] = { 1.0, 1.0, 0.0, 0.0 };
    float ytexvals[] = { 1.0, 0.0, 0.0, 1.0 };

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 4; i++) {
        glVertex2f(xvals[i], yvals[i]);
        glTexCoord2f(xtexvals[i], ytexvals[i]);
    }
    glEnd();
    */
    //glEnd();

    //glPopMatrix();

    glFlush();
    SwapBuffers(hDC);
    //glPopMatrix();
}



GLint compileShader(const char* filename, GLenum type) {

    FILE* file;
    
    fopen_s(&file, filename, "rb");

    if (file == NULL) {
        std::cerr << "Cannot open shader " << filename << std::endl;
        abort();
    }

    fseek(file, 0, SEEK_END);
    const int size = ftell(file);
    rewind(file);

    const GLchar* source = new GLchar[size + 1];
    fread(const_cast<char*>(source), sizeof(char), size, file);
    const_cast<char&>(source[size]) = '\0';

    const GLint shader = glCreateShader(type);

    if (not shader) {
        std::cerr << "Cannot create a shader of type " << shader << std::endl;
        abort();
    }

    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    {
        GLint compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (not compiled) {
            std::cerr << "Cannot compile shader " << filename << std::endl;
            abort();
        }
    }

    return shader;

}

GLint compileShaderProgram(const char* vertexShaderFilename, const char* fragmentShaderFilename) {

    const GLint program = glCreateProgram();

    if (not program) {
        std::cerr << "Cannot create a shader program" << std::endl;
        abort();
    }

    glAttachShader(program, compileShader(vertexShaderFilename, GL_VERTEX_SHADER));
    glAttachShader(program, compileShader(fragmentShaderFilename, GL_FRAGMENT_SHADER));

    glLinkProgram(program);

    {
        GLint linked;
        glGetShaderiv(program, GL_LINK_STATUS, &linked);
        if (not linked) {
            std::cerr << "Cannot link shader program with shaders " << vertexShaderFilename << " and " << fragmentShaderFilename << std::endl;
            abort();
        }
    }

    return program;

}

void CleanupAnimation()
{
    //didn't create any objects, so no need to clean them up
}