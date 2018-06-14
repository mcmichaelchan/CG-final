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
Camera camera(glm::vec3(50.0f, 1.5f, 50.0f));
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

	// Setup ImGui bindings
	/*
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	ImGui_ImplGlfwGL3_Init(window, true);
	ImGui::StyleColorsDark();
	*/

	// Setup and compile our shaders
	Shader shader("shader/vShaderSrc.txt", "shader/fShaderSrc.txt");
	Shader skyboxShader("shader/skyboxVs.txt", "shader/skyboxFrag.txt");

  // read map from files and calculate terrain data
  vector<vector<int>> map = readMap("texture/map3.bmp");
  int size = map.size() * map[0].size();
  float *vertices = new float[size * 8];
  unsigned int *triangleIndices = new unsigned int[size * 6];
  terrainVertexCalculation(vertices, triangleIndices, map, size);	

	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//glBufferStorage(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*size * 8, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*(size * 6), triangleIndices, GL_STATIC_DRAW);
	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	//texture
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);


	// Setup skybox VAO
	GLuint skyboxVAO, skyboxVBO;
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
	int width, height, nrChannels;
	unsigned char *data = SOIL_load_image("texture/sand_texture3.jpg", &width, &height, &nrChannels, 0);

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	SOIL_free_image_data(data);

  // depth test
	glEnable(GL_DEPTH_TEST);

	// ---- render loop ----
	while (!glfwWindowShouldClose(window)) {
		glDepthFunc(GL_LESS); // Set depth function back to default
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glm::mat4 model(1);
		glm::mat4 skyboxModel(1);
		glm::mat4 view(1);
		glm::mat4 projection(1);

		// Process input from keyboard/mouse/other
		processInput(window);

		// init ImGui
		//ImGui_ImplGlfwGL3_NewFrame();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);

    // Draw terrain.
		shader.use();
    model = glm::scale(model, glm::vec3(100.0f, 100.0f, 100.0f));
    model = glm::rotate(model, 90.0f, glm::vec3(1.0, 0.0, 0.0));
		projection = glm::perspective(camera.Zoom, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.01f, 100.0f);
		view = camera.GetViewMatrix();
		//projection = glm::perspective(180.0f, 1.0f, 0.01f, 100.0f);
		shader.setVec3("viewPos", camera.Position.x, camera.Position.y, camera.Position.z);
    shader.setMat4("model", model);
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);

		//glDrawArrays(GL_TRIANGLES, 0, );
		glBindTexture(GL_TEXTURE_2D, texture);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, size * 10, GL_UNSIGNED_INT, 0);



		// Draw skybox at last
		glDepthFunc(GL_LEQUAL);  // Change depth function so depth test passes when values are equal to depth buffer's content
		skyboxShader.use();
    skyboxModel = glm::translate(skyboxModel, glm::vec3(50.0f, -5.0f, 50.0f));
    skyboxModel = glm::scale(skyboxModel, glm::vec3(50.0f, 50.0f, 50.0f));

    //view = glm::mat4(glm::mat3(camera.GetViewMatrix()));	// Remove any translation component of the view matrix
    skyboxShader.setMat4("model", skyboxModel);
    skyboxShader.setMat4("view", view);
    skyboxShader.setMat4("projection", projection);

		// skybox cube
		glBindVertexArray(skyboxVAO);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 30);
		glBindVertexArray(0);
		//glDepthFunc(GL_LESS); // Set depth function back to default

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	// release sources
	//ImGui_ImplGlfwGL3_Shutdown();
	//ImGui::DestroyContext();
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

void terrainVertexCalculation(float *vertices,unsigned int *triangleIndices, vector<vector<int>> map, int size) {
  for (int i = 0, j = 0, k = 0; i < size * 8; i += 8) {
    vertices[i] = (float)j / (float)map[0].size();
    vertices[i + 1] = (float)k / (float)map.size();
    vertices[i + 2] = (float)map[j][k] / 256.0f / 8;
    vertices[i + 3] = 1.0f;
    vertices[i + 4] = 1.0f;
    vertices[i + 5] = 1.0f;
    vertices[i + 6] = (float)j / (float)map[0].size() * 50;
    vertices[i + 7] = (float)k / (float)map.size() * 50;
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