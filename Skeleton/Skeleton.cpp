//=============================================================================================
// Mintaprogram: Z�ld h�romsz�g. Ervenyes 2019. osztol.
//
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat, BOM kihuzando.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kiveve
// - Mashonnan atvett programresszleteket forrasmegjeloles nelkul felhasznalni es
// - felesleges programsorokat a beadott programban hagyni!!!!!!! 
// - felesleges kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan OpenGL fuggvenyek hasznalhatok, amelyek az oran a feladatkiadasig elhangzottak 
// A keretben nem szereplo GLUT fuggvenyek tiltottak.
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : Seregélyes Viktória
// Neptun : OUK8CF
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================
#include "framework.h"

const char* const vertexSource = R"(
    #version 330
    precision highp float;

    layout(location = 0) in vec4 position;

    void main() {
        gl_Position = position;
    }
)";

const char* const fragmentSource = R"(
    #version 330
    precision highp float;

    out vec4 outColor;

    void main() {
        outColor = vec4(1, 0, 0, 1);
    }
)";

GPUProgram gpuProgram(false);
unsigned int vao;
int frame = 0;

void onInitialization() {
    glViewport(0, 0, windowWidth, windowHeight);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    float vertices[] = { -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

    gpuProgram.create(vertexSource, fragmentSource, "outColor");
}

#include <fstream>
#include <sstream>

void onDisplay() {
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    std::string newVertexSrc;
    std::string newFragmentSrc;
    std::string line;
    std::ifstream vfile("C:\\Users\\User\\Desktop\\Egyetem\\4. félév\\Grafika\\GrafikaHaziCsomag3\\GrafikaHaziCsomag\\Programs\\Skeleton\\Skeleton\\vertex.vert");
    while (std::getline(vfile, line)) {
        newVertexSrc += line + "\n";
    }
    vfile.close();
    std::ifstream ffile("C:\\Users\\User\\Desktop\\Egyetem\\4. félév\\Grafika\\GrafikaHaziCsomag3\\GrafikaHaziCsomag\\Programs\\Skeleton\\Skeleton\\fragment.frag");
    while (std::getline(ffile, line)) {
        newFragmentSrc += line + "\n";
    }
    ffile.close();

    GPUProgram gpuProgram(false);
    gpuProgram.create(newVertexSrc.c_str(), newFragmentSrc.c_str(), "outColor");

    gpuProgram.setUniform((float)frame, "frame");
    frame++;

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glutSwapBuffers();
    glutPostRedisplay();
}

void onKeyboard(unsigned char key, int pX, int pY) {
}

void onKeyboardUp(unsigned char key, int pX, int pY) {
}

void onMouseMotion(int pX, int pY) {
}

void onMouse(int button, int state, int pX, int pY) {
}

void onIdle() {
}
