#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw_gl3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader/shader.h>
#include <camera/camera.h>

#include <cimg/CImg.h>

#include <SOIL2\SOIL2.h>

#include <texture/Texture.h>

using namespace std;

#define WINDOW_WIDTH  1600  
#define WINDOW_HEIGHT 800

// global variables
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
//Camera camera(glm::vec3(49.0f, 39.0f, -48.0f)); // LIGHTPOS
//Camera camera(glm::vec3(0.0f, 0.0f, 0.0f)); // origin
vector<vector<int>> m_textures;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastX = (float)WINDOW_WIDTH / 2.0;
float lastY = (float)WINDOW_HEIGHT / 2.0;
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
vector<vector<int>> readMap(const char *path);
void terrainVertexCalculation(float *vertices, unsigned int *triangleIndices, vector<vector<int>> map, int size);
glm::vec3 crossProduct(glm::vec3 a, glm::vec3 b);
glm::mat4 lightSpaceMatrix;

//阴影贴图相关函数
void DepthMap(Shader shader1, Shader shader2);
//场景渲染，在此具体指添加图元
void renderScene(Shader shader);

//各种缓冲
unsigned int VAO, VBO, EBO;
GLuint skyboxVAO, skyboxVBO;

//光源位置
glm::vec3 lightPos = glm::vec3(49.0f, 39.0f, -48.0f);
//深度贴图帧缓冲
GLuint depthMapFBO;
//2D纹理
GLuint depthMap;
GLuint terrainTexture;
GLuint normalMap;

//2D纹理大小
const GLuint SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;

//地图大小
int mapsize;

glm::mat4 model;
glm::mat4 skyboxModel;
glm::mat4 view;
glm::mat4 projection;

// skybox vertices
const GLfloat skyboxVertices[] = {
  // Positions
  -1.0f,  1.0f, -1.0f,
  -1.0f, -1.0f, -1.0f,
  1.0f, -1.0f, -1.0f,
  1.0f, -1.0f, -1.0f,
  1.0f,  1.0f, -1.0f,
  -1.0f,  1.0f, -1.0f,

  -1.0f, -1.0f,  1.0f,
  -1.0f, -1.0f, -1.0f,
  -1.0f,  1.0f, -1.0f,
  -1.0f,  1.0f, -1.0f,
  -1.0f,  1.0f,  1.0f,
  -1.0f, -1.0f,  1.0f,

  1.0f, -1.0f, -1.0f,
  1.0f, -1.0f,  1.0f,
  1.0f,  1.0f,  1.0f,
  1.0f,  1.0f,  1.0f,
  1.0f,  1.0f, -1.0f,
  1.0f, -1.0f, -1.0f,

  -1.0f, -1.0f,  1.0f,
  -1.0f,  1.0f,  1.0f,
  1.0f,  1.0f,  1.0f,
  1.0f,  1.0f,  1.0f,
  1.0f, -1.0f,  1.0f,
  -1.0f, -1.0f,  1.0f,

  -1.0f,  1.0f, -1.0f,
  1.0f,  1.0f, -1.0f,
  1.0f,  1.0f,  1.0f,
  1.0f,  1.0f,  1.0f,
  -1.0f,  1.0f,  1.0f,
  -1.0f,  1.0f, -1.0f,

  -1.0f, -1.0f, -1.0f,
  -1.0f, -1.0f,  1.0f,
  1.0f, -1.0f, -1.0f,
  1.0f, -1.0f, -1.0f,
  -1.0f, -1.0f,  1.0f,
  1.0f, -1.0f,  1.0f
};



int main() {
	glfwInit();
	// OpenGL version & mode setting
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a window & context/viewpoint setting
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Final OpenGL Project", NULL, NULL);
	if (window == NULL) {
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();
		return -1;
	}

  // Callback functions
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad init
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		cout << "Failed to initialize GLAD" << endl;
		return -1;
	}

	// Setup and compile our shaders
	Shader shader("shader/vShaderSrc.txt", "shader/fShaderSrc.txt");
	Shader skyboxShader("shader/skyboxVs.txt", "shader/skyboxFrag.txt");

	Shader shader_simple("shader/vShaderSrc_simple.txt", "shader/fShaderSrc_simple.txt");
	Shader shader_noe("shader/vShaderSrc_noe.txt", "shader/fShaderSrc_noe.txt");
	Shader shader_shadow("shader/vShaderSrc_shadow.txt", "shader/fShaderSrc_shadow.txt");
	Shader shader_light("shader/vShaderSrc_light.txt", "shader/fShaderSrc_light.txt");

  // read map from files and calculate terrain data
  vector<vector<int>> map = readMap("texture/map3.bmp");
  mapsize = map.size() * map[0].size();
  float *vertices = new float[mapsize * 10];
  unsigned int *triangleIndices = new unsigned int[mapsize * 6];
  terrainVertexCalculation(vertices, triangleIndices, map, mapsize);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//glBufferStorage(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*mapsize * 10, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*(mapsize * 6), triangleIndices, GL_STATIC_DRAW);
	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	//texture
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
  // normal map
  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(8 * sizeof(float)));
  glEnableVertexAttribArray(3);

  glBindVertexArray(0);

	// Setup skybox VAO
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *)0);
	glBindVertexArray(0);


	// Load Textures
	// Cubemap (Skybox)
	vector<const GLchar*> faces;
	faces.push_back("skybox/sahara/right.tga");
	faces.push_back("skybox/sahara/left.tga");
	faces.push_back("skybox/sahara/top.tga");
	faces.push_back("skybox/sahara/bottom.tga");
	faces.push_back("skybox/sahara/back.tga");
	faces.push_back("skybox/sahara/front.tga");
	GLuint cubemapTexture = TextureLoading::LoadCubemap(faces);

	// load terrain texture
  terrainTexture = TextureLoading::LoadTexture((GLchar*)"texture/sand_texture3.jpg");
  // load normal map texture
  normalMap = TextureLoading::LoadTexture((GLchar*)"texture/sand_normal.jpg");

	//---depthmap
	glGenFramebuffers(1, &depthMapFBO);
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// shader configuration
	// --------------------
	shader_shadow.use();
	shader_shadow.setInt("terrainTexture", 0);
	shader_shadow.setInt("depthMap", 1);
  shader_shadow.setInt("normalMap", 2);

  // depth test
	glEnable(GL_DEPTH_TEST);

	// ---- render loop ----
	while (!glfwWindowShouldClose(window)) {
		glDepthFunc(GL_LESS); // Set depth function back to default
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		// Process input from keyboard/mouse/other
		processInput(window);

		// init ImGui
		//ImGui_ImplGlfwGL3_NewFrame();

    lightPos = camera.Position;

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);

		model = glm::mat4(1);
		projection = glm::mat4(1);
		view = glm::mat4(1);
		skyboxModel = glm::mat4(1);

		projection = glm::perspective(camera.Zoom, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.01f, 100.0f);
		view = camera.GetViewMatrix();
		
    //scene rendered here
	  DepthMap(shader_simple, shader_shadow);

		// Draw skybox at last
		glDepthFunc(GL_LEQUAL);  // Change depth function so depth test passes when values are equal to depth buffer's content
		skyboxShader.use();
    skyboxModel = glm::rotate(skyboxModel, 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
		skyboxModel = glm::translate(skyboxModel, glm::vec3(50.0f, 0.0f, -50.0f));
		skyboxModel = glm::scale(skyboxModel, glm::vec3(50.0f, 50.0f, 50.0f));

		//view = glm::mat4(glm::mat3(camera.GetViewMatrix()));	// Remove any translation component of the view matrix
		skyboxShader.setMat4("model", skyboxModel);
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);
    skyboxShader.setInt("skybox", 0);
		// skybox cube
    
		glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 30);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // Set depth function back to default


		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	// release sources
	//ImGui_ImplGlfwGL3_Shutdown();
	//ImGui::DestroyContext();
  delete vertices;
  delete triangleIndices;
	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
	// press ESC to close
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	float cameraSpeed = 2.5 * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.ProcessKeyboard(FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;
	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(yoffset);
}

vector<vector<int>> readMap(const char *path) {
	cimg_library::CImg<unsigned char> img(path);
	//img.display();
	int width = img.width();
	int height = img.height();
	vector<int> row(height, 0);
	vector<vector<int>> map(width, row);
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			map[i][j] = (int)img(i, j, 0);
		}
	}
	return map;
}

glm::vec3 crossProduct(glm::vec3 a, glm::vec3 b) {
	return glm::vec3{ (float)(a.y*b.z) - (float)(b.y*a.z), (float)(b.x*a.z) - (float)(a.x*b.z), (float)(a.x*b.y) - (float)(b.x*a.y) };
}

void terrainVertexCalculation(float *vertices,unsigned int *triangleIndices, vector<vector<int>> map, int size) {
  for (int i = 0, j = 0, k = 0; i < size * 10; i += 10) {
    vertices[i] = (float)j / (float)map[0].size();
    vertices[i + 1] = (float)k / (float)map.size();
    vertices[i + 2] = (float)map[j][k] / 256.0f / 10;

	//法向量
	glm::vec3 v0 = glm::vec3{ (float)j / (float)map[0].size(), (float)k / (float)map.size(), (float)map[j][k] / 256.0f / 8 };
	glm::vec3 v1;
	glm::vec3 v2;
	glm::vec3 v3;
	glm::vec3 v4;
	glm::vec3 a;
	glm::vec3 b;
	
	//v0v1v2
	glm::vec3 n1 = glm::vec3{ 0.0f,0.0f,0.0f };
	if (k - 1 < 0 || j - 1 < 0) {
		n1 = glm::vec3{ 0.0f,0.0f,0.0f };
	}
	else {
		v1 = glm::vec3{ (float)j / (float)map[0].size(), (float)(k - 1) / (float)map.size(), (float)map[j][k - 1] / 256.0f / 8 };
		v2 = glm::vec3{ (float)(j - 1) / (float)map[0].size(), (float)k / (float)map.size(), (float)map[j - 1][k] / 256.0f / 8 };
		a = v1 - v0;
		b = v2 - v0;
		n1 = crossProduct(a, b);
	}

	//v0v2v3
	glm::vec3 n2 = glm::vec3{ 0.0f,0.0f,0.0f };
	if (k + 1 > (map.size() - 1) || j - 1 < 0) {
		n2 = glm::vec3{ 0.0f,0.0f,0.0f };
	}
	else {
		v3 = glm::vec3{ (float)j / (float)map[0].size(), (float)(k + 1) / (float)map.size(), (float)map[j][k + 1] / 256.0f / 8 };
		a = v2 - v0;
		b = v3 - v0;
		n2 = crossProduct(a, b);
	}

	//v0v3v4
	glm::vec3 n3 = glm::vec3{ 0.0f,0.0f,0.0f };
	if (k + 1 > (map.size() - 1) || j + 1 > (map[0].size() - 1)) {
		n3 = glm::vec3{ 0.0f,0.0f,0.0f };
	}
	else {
		v4 = glm::vec3{ (float)(j + 1) / (float)map[0].size(), (float)k / (float)map.size(), (float)map[j + 1][k] / 256.0f / 8 };
		a = v3 - v0;
		b = v4 - v0;
		n3 = crossProduct(a, b);
	}

	//v0v4v1
	glm::vec3 n4 = glm::vec3{ 0.0f,0.0f,0.0f };
	if (k - 1 < 0 || j + 1 > (map[0].size() - 1)) {
		n4 = glm::vec3{ 0.0f,0.0f,0.0f };
	}
	else {
		a = v4 - v0;
		b = v1 - v0;
		n4 = crossProduct(a, b);
	}

	//相加归一
	glm::vec3 Nor = n1 + n2 + n3 + n4;

    vertices[i + 3] = Nor.x;
    vertices[i + 4] = Nor.y;
    vertices[i + 5] = Nor.z;

    vertices[i + 6] = (float)j / (float)map[0].size() * 50;
    vertices[i + 7] = (float)k / (float)map.size() * 50;
    vertices[i + 8] = (float)j / (float)map[0].size() * 100;
    vertices[i + 9] = (float)k / (float)map.size() * 100;
    k++;
    if (k >= map[0].size()) {
      k = 0;
      j++;
    }
  }
  int count = 0;
  for (int i = 0; i < size - map[0].size(); i++) {
    if (i % map[0].size() == map[0].size() - 1) {
      continue;
    }
    triangleIndices[count] = i;
    triangleIndices[count + 1] = i + 1;
    triangleIndices[count + 2] = i + map[0].size();
    triangleIndices[count + 3] = i + 1;
    triangleIndices[count + 4] = i + map[0].size();
    triangleIndices[count + 5] = i + map[0].size() + 1;
    count += 6;
  }
}

void renderScene(Shader shader) {
	// Draw terrain.
	shader.setMat4("model", model);
	glBindVertexArray(VAO);
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElements(GL_TRIANGLES, mapsize * 10, GL_UNSIGNED_INT, 0);
  //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindVertexArray(0);
}

void DepthMap(Shader shader1, Shader shader2) {
	model = glm::scale(model, glm::vec3(100.0f, 100.0f, 100.0f));
	//model = glm::rotate(model, 90.0f, glm::vec3(1.0, 0.0, 0.0));

	GLfloat near_plane =1.0f, far_plane = 100.0f;
	//glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
	//glm::mat4 lightView = glm::lookAt(
	//	lightPos, // 
	//	glm::vec3(0.0f, 0.0f, 0.0f), // and looks at the origin
	//	glm::vec3(0.0f, 1.0f, 0.0f) // Head is up (set to 0,-1,0 to look upside-down)		
	//);

  //glm::mat4 lightProjection = glm::perspective(45.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.01f, 100.0f);
  //glm::mat4 lightView = glm::lookAt(
  //  glm::vec3(49.0f, 39.0f, -48.0f),
  //  glm::vec3(49.0f, 39.0f, -48.0f) + glm::vec3(0.0f, -0.4f, 0.9f),
  //  glm::vec3(0.0f, 1.0f, 0.4f)
  //);

  glm::mat4 lightProjection = glm::perspective(camera.Zoom, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.01f, 100.0f);
  glm::mat4 lightView = camera.GetViewMatrix();

	lightSpaceMatrix = lightProjection * lightView;
	
	shader1.use();
	shader1.setMat4("lightSpaceMatrix", lightSpaceMatrix);

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	renderScene(shader1);
  
	//渲染后解绑framebuffer并用回原来的shader
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader2.use();

	shader2.setMat4("view", view);
	shader2.setMat4("projection", projection);
	shader2.setVec3("lightPos", lightPos.x, lightPos.y, lightPos.z);
	shader2.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
	shader2.setVec3("viewPos", camera.Position.x, camera.Position.y, camera.Position.z);
	shader2.setMat4("lightSpaceMatrix", lightSpaceMatrix);

  glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, terrainTexture);
	
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, normalMap);

  glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);


	renderScene(shader2);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
}
