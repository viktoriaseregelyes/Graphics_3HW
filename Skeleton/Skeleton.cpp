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

class PhongShader : public GPUProgram {
	const char * vertexSource = R"(
		#version 330
		precision highp float;

		uniform mat4  MVP, M, Minv; // MVP, Model, Model-inverse
		uniform vec3  wLiDir;       // light source direction 
		uniform vec3  wEye;         // pos of eye

		layout(location = 0) in vec3  vtxPos;            // pos in modeling space
		layout(location = 1) in vec3  vtxNorm;      	 // normal in modeling space
		layout(location = 2) in vec2  vtxUV;

		out vec3 wNormal;		    // normal in world space
		out vec3 wView;             // view in world space
		out vec3 wLight;		    // light dir in world space

		void main() {
		   gl_Position = vec4(vtxPos, 1) * MVP; // to NDC
		   vec4 wPos = vec4(vtxPos, 1) * M;
		   wLight  = wLiDir;
		   wView   = wEye - wPos.xyz;
		   wNormal = (Minv * vec4(vtxNorm, 0)).xyz;
		}
	)";

	const char * fragmentSource = R"(
		#version 330
		precision highp float;

		uniform vec3 kd, ks, ka; // diffuse, specular, ambient ref
		uniform vec3 La, Le;     // ambient and point sources
		uniform float shine;     // shininess for specular ref

		in  vec3 wNormal;       // interpolated world sp normal
		in  vec3 wView;         // interpolated world sp view
		in  vec3 wLight;        // interpolated world sp illum dir
		in vec2 texcoord;
		out vec4 fragmentColor; // output goes to frame buffer

		void main() {
			vec3 N = normalize(wNormal);
			vec3 V = normalize(wView); 
			vec3 L = normalize(wLight);
			vec3 H = normalize(L + V);
			float cost = max(dot(N,L), 0), cosd = max(dot(N,H), 0);
			vec3 color = ka * La + (kd * cost + ks * pow(cosd,shine)) * Le;
			fragmentColor = vec4(color, 1);
		}
	)";
public:
	PhongShader() { create(vertexSource, fragmentSource, "fragmentColor"); }
};

PhongShader *gpuProgram;

struct Camera {
	vec3 wEye, wLookat, wVup;
	float fov, asp, fp, bp;
public:
	Camera() {
		asp = 1;
		fov = 80.0f * (float)M_PI / 180.0f;
		fp = 0.1f; bp = 100;
	}
	mat4 V() {
		vec3 w = normalize(wEye - wLookat);
		vec3 u = normalize(cross(wVup, w));
		vec3 v = cross(w, u);
		return TranslateMatrix(-wEye) * mat4(u.x, v.x, w.x, 0,
			u.y, v.y, w.y, 0,
			u.z, v.z, w.z, 0,
			0, 0, 0, 1);
	}
	mat4 P() {
		return mat4(1 / (tanf(fov / 2)*asp), 0, 0, 0,
					0, 1 / tanf(fov / 2), 0, 0,
					0, 0, -(fp + bp) / (bp - fp), -1,
					0, 0, -2 * fp*bp / (bp - fp), 0);
	}
	void SetUniform() {
		int location = glGetUniformLocation(gpuProgram->getId(), "wEye");
		if (location >= 0) glUniform3fv(location, 1, &wEye.x);
		else printf("uniform wEye cannot be set\n");
	}
};

Camera camera;

struct Material {
	vec3 kd, ks, ka;
	float shininess;

	void SetUniform() {
		gpuProgram->setUniform(kd, "kd");
		gpuProgram->setUniform(kd, "ks");
		gpuProgram->setUniform(kd, "ka");
		int location = glGetUniformLocation(gpuProgram->getId(), "shine");
		if (location >= 0) glUniform1f(location, shininess); else printf("uniform shininess cannot be set\n");
	}
};

struct Light {
	vec3 La, Le;
	vec3 wLightDir;

	Light() : La(1, 1, 1), Le(3, 3, 3) { }

	void SetUniform(bool enable) {
		if (enable) {
			gpuProgram->setUniform(La, "La");
			gpuProgram->setUniform(Le, "Le");
		}
		else {
			gpuProgram->setUniform(vec3(0, 0, 0), "La");
			gpuProgram->setUniform(vec3(0, 0, 0), "Le");
		}
		gpuProgram->setUniform(wLightDir, "wLiDir");
	}
};

struct VertexData {
	vec3 position, normal;
	vec2 texcoord;
};

class Geometry {
	unsigned int vao, type;
protected:
	int nVertices;
public:
	Geometry(unsigned int _type) {
		type = _type;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}
	void Draw(mat4 M, mat4 Minv) {
		mat4 MVP = M * camera.V() * camera.P();
		gpuProgram->setUniform(MVP, "MVP");
		gpuProgram->setUniform(M, "M");
		gpuProgram->setUniform(Minv, "Minv");
		glBindVertexArray(vao);
		glDrawArrays(type, 0, nVertices);
	}
};

class ParamSurface : public Geometry {
public:
	ParamSurface() : Geometry(GL_TRIANGLES) {}

	virtual VertexData GenVertexData(float u, float v) = 0;

	void Create(int N = 16, int M = 16) {
		unsigned int vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		nVertices = N * M * 6;
		std::vector<VertexData> vtxData;
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < M; j++) {
				vtxData.push_back(GenVertexData((float)i / N, (float)j / M));
				vtxData.push_back(GenVertexData((float)(i + 1) / N, (float)j / M));
				vtxData.push_back(GenVertexData((float)i / N, (float)(j + 1) / M));
				vtxData.push_back(GenVertexData((float)(i + 1) / N, (float)j / M));
				vtxData.push_back(GenVertexData((float)(i + 1) / N, (float)(j + 1) / M));
				vtxData.push_back(GenVertexData((float)i / N, (float)(j + 1) / M));
			}
		}
		glBufferData(GL_ARRAY_BUFFER, nVertices * sizeof(VertexData), &vtxData[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, position));
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, normal));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, texcoord));
	}
};

class Sphere : public ParamSurface {
	float r;
public:
	Sphere(float _r) {
		r = _r;
		Create(20, 20);
	}

	VertexData GenVertexData(float u, float v) {
		VertexData vd;
		vd.normal = vec3(cosf(u * 2.0f * M_PI) * sinf(v*M_PI), sinf(u * 2.0f * M_PI) * sinf(v*M_PI), cosf(v*M_PI));
		vd.position = vd.normal * r;
		vd.texcoord = vec2(u, v);
		return vd;
	}
};

class Conic : public ParamSurface {
	float r;
public:
	Conic(float _r) {
		r = _r;
		Create(20, 20);
	}

	VertexData GenVertexData(float u, float v) {
		VertexData vd;
		float U = u * 2 * M_PI;
		vd.normal = vec3(cosf(U) * v, sinf(U) * v, 0);
		vd.position = vd.normal * r;
		vd.texcoord = vec2(u, v);
		return vd;
	}
};

class Cylinder : public ParamSurface {
	float r;
	float height;
public:
	Cylinder(float _r, float _height) {
		r = _r;
		height = _height;
		Create(20, 20);
	}

	VertexData GenVertexData(float u, float v) {
		VertexData vd;
		float U = u * 2 * M_PI, V = v * height;
		vd.normal = vec3(cos(U) * r, sin(U) * r, 0);
		vd.position = vd.normal + vec3(0, 0, V);
		vd.texcoord = vec2(u, v);
		return vd;
	}
};

class Paraboloid : public ParamSurface {
	float r;
	float height;
public:
	Paraboloid(float _r, float _height) {
		r = _r;
		height = _height;
		Create(20, 20);
	}

	VertexData GenVertexData(float u, float v) {
		VertexData vd;
		float U = u * 2 * M_PI, V = v * height;
		vd.normal = vec3(sqrtf(v * M_PI) * sinf(U) * r, sqrtf(v * M_PI) * cosf(U) * r, v * M_PI);
		vd.position = vd.normal + vec3(0, 0, V);
		vd.texcoord = vec2(u, v);
		return vd;
	}
};

class Floor {
	Material* material;
public:
	Floor(Material* _m) {
		material = _m;
	}
	void Draw(mat4 M, mat4 Minv) {
		material->SetUniform();
	}
};

class Lamp {
	Material* material;
	Cylinder* feet;
	Sphere* joint;
	Cylinder* arm;
	Conic* con;
	Paraboloid* para;

	float angleDown = 270;
	boolean aDown = false;
	float angleUp = 340;
	boolean aUp = false;
	float angleHead = 300;
	boolean aHead = false;
	float angleFull = 300;
	boolean aFull = false;
	
public:
	Lamp(Material* _m) {
		material = _m;
		arm = new Cylinder(0.5f, 10.0f);
		feet = new Cylinder(6.0f, 1.0f);
		joint = new Sphere(0.8f);
		con = new Conic(6.0f);
		para = new Paraboloid(4.0f, 3.0f);
	}

	void Animate(float dt) {
		if (!aDown) {
			angleDown += dt;
			AngleUp(dt);
			if (angleDown >= 360) {
				aDown = true;
			}
		}
		else if (aDown) {
			angleDown -= dt;
			AngleUp(dt);
			if (angleDown <= 240) {
				aDown = false;
			}
		}
		AngleFull(dt);
	}

	void AngleUp(float dt) {
		if (!aUp) {
			angleUp += dt;
			AngleHead(dt);
			if (angleUp >= 360) {
				aUp = true;
			}
		}
		else if (aUp) {
			angleUp -= dt;
			AngleHead(dt);
			if (angleUp <= 240) {
				aUp = false;
			}
		}
	}

	void AngleHead(float dt) {
		if (!aHead) {
			angleHead += dt;
			if (angleHead >= 360) {
				aHead = true;
			}
		}
		else if (aHead) {
			angleHead -= dt;
			if (angleHead <= 240) {
				aHead = false;
			}
		}
	}

	void AngleFull(float dt) {
		if (!aFull) {
			angleFull += dt;
			if (angleFull >= 300) {
				aFull = true;
			}
		}
		else if (aFull) {
			angleFull -= dt;
			if (angleFull <= 300) {
				aFull = false;
			}
		}
	}

	void DrawHead(mat4 M, mat4 Minv) {
		joint->Draw(M, Minv);

		M = TranslateMatrix(vec3(0, 0, 0.8f)) * RotationMatrix(270 * M_PI / 180, vec3(1.0f, 0, 0)) * M;
		Minv = Minv * RotationMatrix(-270 * M_PI / 180, vec3(1, 0, 0)) * TranslateMatrix(vec3(0, 0, 0.8f));
		para->Draw(M, Minv);
	}

	void DrawFeet(mat4 M, mat4 Minv) {
		M = ScaleMatrix(vec3(1.0f, 1.0f, 1.2f)) * RotationMatrix(90 * M_PI / 180, vec3(1.0f, 0, 0)) * M;
		Minv = Minv * RotationMatrix(-90 * M_PI / 180, vec3(1.0f, 0, 0)) * ScaleMatrix(vec3(1.0f, 1.0f, 1.2f));
		feet->Draw(M, Minv);
		con->Draw(M, Minv);
	}

	void DrawUp(mat4 M, mat4 Minv) {
		//light.wLightDir = vec3(5, 5, 4);
		//light.SetUniform(true);

		joint->Draw(M, Minv);

		M = RotationMatrix(angleDown * M_PI / 180, vec3(1.0f, 0, 0)) * M;
		Minv = Minv * RotationMatrix(-angleDown * M_PI / 180, vec3(1.0f, 0, 0));
		arm->Draw(M, Minv);

		M = TranslateMatrix(vec3(0, 0, 10)) * M;
		Minv = Minv * TranslateMatrix(-vec3(0, 0, 10));
		joint->Draw(M, Minv);

		M = RotationMatrix(angleUp * M_PI / 180, vec3(1.0f, 0, 0)) * RotationMatrix(angleFull * M_PI / 180, vec3(0, 1.0f, 0)) * M;
		Minv = Minv * RotationMatrix(-angleUp * M_PI / 180, vec3(1.0f, 0, 0)) * RotationMatrix(-angleFull * M_PI / 180, vec3(0, 1.0f, 0));
		arm->Draw(M, Minv);

		M = TranslateMatrix(vec3(0, 0, 10)) * M;
		Minv = Minv * TranslateMatrix(-vec3(0, 0, 10));
		joint->Draw(M, Minv);

		M = RotationMatrix(angleHead * M_PI / 180, vec3(1.0f, 0, 0)) * RotationMatrix(angleFull * M_PI / 180, vec3(0, 1.0f, 0)) * M;
		Minv = Minv * RotationMatrix(-angleHead * M_PI / 180, vec3(1.0f, 0, 0)) * RotationMatrix(-angleFull * M_PI / 180, vec3(0, 1.0f, 0));
		para->Draw(M, Minv);
	}

	void Draw(mat4 M, mat4 Minv) {
		M = TranslateMatrix(vec3(0, 1.2f, 0)) * M;
		Minv = Minv * TranslateMatrix(-vec3(0, 1.2f, 0));
		material->SetUniform();
		DrawFeet(M, Minv);

		M = TranslateMatrix(vec3(0, 0.8f, 0)) * M;
		Minv = Minv * TranslateMatrix(-vec3(0, 0.8f, 0));
		material->SetUniform();
		DrawUp(M, Minv);
	}
};

class Scene {
	Lamp *lamp;
	Floor *floor;
public:
	Light light;
	Light lamp_light;

	void Build() {
		Material *material0 = new Material;
		material0->kd = vec3(0.21f, 0.23f, 0.25f);
		material0->ks = vec3(1, 1, 1);
		material0->ka = vec3(0.2f, 0.3f, 1);
		material0->shininess = 20;

		Material *material1 = new Material;
		material1->kd = vec3(0, 1, 1);
		material1->ks = vec3(2, 2, 2);
		material1->ka = vec3(0.2f, 0.2f, 0.2f);
		material1->shininess = 200;

		lamp = new Lamp(material0);
		floor = new Floor(material1);

		camera.wEye = vec3(0, 0, 4);
		camera.wLookat = vec3(0, 0, 0);
		camera.wVup = vec3(0, 1, 0);

		light.wLightDir = vec3(5, 5, 4);
	}
	void Render() {
		lamp_light.wLightDir = vec3(0, 10, 0);

		camera.SetUniform();
		light.SetUniform(true);

		mat4 unit = TranslateMatrix(vec3(0, 0, 0));
		floor->Draw(unit, unit);

		lamp->Draw(unit, unit);

		light.SetUniform(false);

		mat4 shadowMatrix = { 1, 0, 0, 0,
							  -light.wLightDir.x / light.wLightDir.y, 0, -light.wLightDir.z / light.wLightDir.y, 0,
							  0, 0, 1, 0,
							  0, 0.001f, 0, 1 };
		lamp->Draw(shadowMatrix, shadowMatrix);

		lamp_light.SetUniform(true);

		lamp_light.SetUniform(false);

		mat4 shadowMatrix2 = { 1, 0, 0, 0,
							  -lamp_light.wLightDir.x / lamp_light.wLightDir.y, 0, -lamp_light.wLightDir.z / lamp_light.wLightDir.y, 0,
							  0, 0, 1, 0,
							  0, 0.001f, 0, 1 };
		lamp->Draw(shadowMatrix2, shadowMatrix2);
	}

	void Animate(float t) {
		static float tprev = 0;
		float dt = t - tprev;
		tprev = t;

		lamp->Animate(dt);

		static float cam_angle = 0;
		cam_angle += 0.01f * dt;

		const float camera_rad = 35;
		camera.wEye = vec3(cos(cam_angle) * camera_rad, 15, sin(cam_angle) * camera_rad);
		camera.wLookat = vec3(0, 15, 0);
	}
};

unsigned int vao;
Scene scene;

void onInitialization() {
	glViewport(0, 0, windowWidth, windowHeight);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	gpuProgram = new PhongShader();
	scene.Build();
}

void onDisplay() {
	glClearColor(0.37f, 0.27f, 0.25f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	scene.Render();
	glutSwapBuffers();
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
	long time = glutGet(GLUT_ELAPSED_TIME);
	float sec = time / 30.0f;
	scene.Animate(sec);
	glutPostRedisplay();
}