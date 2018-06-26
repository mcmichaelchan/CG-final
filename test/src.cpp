#include <iostream>
#include <string>

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

#include "particals.h"
#include "model.h"

#include <SOIL2\SOIL2.h>

#include <texture/Texture.h>
#include <map>
#include <ft2build.h>
#include FT_FREETYPE_H

using namespace std;

#define WINDOW_WIDTH  1600  
#define WINDOW_HEIGHT 800

// global variables
//Camera camera(glm::vec3(49.0f, 39.0f, -48.0f)); // LIGHTPOS
Camera camera(glm::vec3(48.0f, 0.0f, 40.0f)); // origin
vector<vector<int>> m_textures;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float *vertices = nullptr;
float lastX = (float)WINDOW_WIDTH / 2.0;
float lastY = (float)WINDOW_HEIGHT / 2.0;
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
vector<vector<int>> readMap(const char *path);
vector<vector<glm::vec3>> terrainVertexCalculation(float *vertices, vector<vector<int>> map, int size);
void addVertice(float *vertices, int offset, glm::vec3 pos, glm::vec3 nm, glm::vec2 txt, glm::vec2 nmmap, glm::vec3 tan, glm::vec3 bitan);
void RenderText(Shader &shader, string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);
void TBNcalculation(glm::vec3 edge1, glm::vec3 edge2, glm::vec2 deltaUV1, glm::vec2 deltaUV2, glm::vec3 &tan, glm::vec3 &bitan);
void nmSum(vector<vector<glm::vec3>> nmArray);
glm::vec3 crossProduct(glm::vec3 a, glm::vec3 b);
glm::mat4 lightSpaceMatrix;


void DepthMap(Shader &shader1, Model &ourModel);

void renderScene(Shader &shader);
void renderModel(Shader &shader1, Model &ourModel);
void renderCube(Shader &shader1);


unsigned int VAO, VBO, EBO;
GLuint skyboxVAO, skyboxVBO;
GLuint cubeVAO, cubeVBO;
GLuint TextVAO, TextVBO;

glm::vec3 lightPos = glm::vec3(48.0f, 17.0f, 3.0f);

GLuint depthMapFBO;

GLuint depthMap;
GLuint terrainTexture;
GLuint normalMap;
GLuint woodTexture;

const GLuint SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;

int mapsize;
int itemNums = 16;

glm::mat4 treeModel;
glm::mat4 cubeModel;
glm::mat4 model;
glm::mat4 skyboxModel;
glm::mat4 view;
glm::mat4 projection;

float cube[] = {
	//top
	0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
	-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
	-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
	-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
	//forward
	0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
	0.5f, -0.5f, 0.5f, 0.0f, 0.0f,1.0f, 0.0f, 1.0f,
	-0.5f, 0.5f,0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
	0.5f, -0.5f, 0.5f, 0.0f, 0.0f,1.0f, 0.0f, 1.0f,
	-0.5f, 0.5f,0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
	-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
	//left
	-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	-0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
	-0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
	-0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
	-0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
	-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	//right
	0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	0.5f, 0.5f, -0.5f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
	0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
	0.5f, 0.5f, -0.5f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
	0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
	0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	//back
	0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
	0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
	-0.5f, 0.5f,-0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
	0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
	-0.5f, 0.5f,-0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
	-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
	//bottom
	0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
	-0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
	0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	-0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
	0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f
};

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

struct Character {
  GLuint TextureID;
  glm::ivec2 Size;
  glm::ivec2 Bearing;
  GLuint Advance;
};
map<GLchar, Character> Characters;

//the position of models
vector<float> deviations_x, deviations_y, deviations_z;

int main() {
	glfwInit();
	// OpenGL version & mode setting
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 4);

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
// Set OpenGL options
	//glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// configure global opengl state
	// -----------------------------
	//glEnable(GL_DEPTH_TEST);

	Shader textShader("shader/text.vs", "shader/text.fs");
	glm::mat4 Projection = glm::ortho(0.0f, static_cast<GLfloat>(WINDOW_WIDTH), 0.0f, static_cast<GLfloat>(WINDOW_HEIGHT));
	textShader.use();
	textShader.setMat4("projection", Projection);

	// FreeType
	FT_Library ft;
	// All functions return a value different than 0 whenever an error occurred
	if (FT_Init_FreeType(&ft))
		cout << "ERROR::FREETYPE: Could not init FreeType Library" << endl;

	// Load font as face
	FT_Face face;
	if (FT_New_Face(ft, "fonts/arial.ttf", 0, &face))
		cout << "ERROR::FREETYPE: Failed to load font" << endl;

	// Set size to load glyphs as
	FT_Set_Pixel_Sizes(face, 0, 48);

	// Disable byte-alignment restriction
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Load first 128 characters of ASCII set
	for (GLubyte c = 0; c < 128; c++)
	{
		// Load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			cout << "ERROR::FREETYTPE: Failed to load Glyph" << endl;
			continue;
		}
		// Generate texture
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// Set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Now store character for later use
		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};
		Characters.insert(pair<GLchar, Character>(c, character));
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	// Destroy FreeType once we're finished
	FT_Done_Face(face);
	FT_Done_FreeType(ft);


	// Configure VAO/VBO for texture quads
	glGenVertexArrays(1, &TextVAO);
	glGenBuffers(1, &TextVBO);
	glBindVertexArray(TextVAO);
	glBindBuffer(GL_ARRAY_BUFFER, TextVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// build and compile shaders
	// -------------------------
	Shader ourShader("shader/model_loading.vs", "shader/model_loading.fs");

	// load models
	// -----------
	Model ourModel("objects/Palm_01/Palm_01.obj");

	// Setup and compile our shaders
	Shader shader("shader/vShaderSrc.txt", "shader/fShaderSrc.txt");
	Shader skyboxShader("shader/skyboxVs.txt", "shader/skyboxFrag.txt");
	Shader particalsShader("shader/vShaderSrcParticals.txt", "shader/fShaderSrcParticals.txt");
	Shader shader_simple("shader/vShaderSrc_simple.txt", "shader/fShaderSrc_simple.txt");
	Shader shader_shadow("shader/vShaderSrc_shadow.txt", "shader/fShaderSrc_shadow.txt");
	Shader shader_light("shader/vShaderSrc_light.txt", "shader/fShaderSrc_light.txt");
	Shader shader_cube("shader/vShaderSrc_cube.txt", "shader/fShaderSrc_cube.txt");
	Shader shader_border("shader/vShaderSrc_border.txt", "shader/fShaderSrc_border.txt");

  // read map from files and calculate terrain data
  vector<vector<int>> map = readMap("texture/map3.bmp");
  mapsize = (map.size() - 1) * (map[0].size() - 1);
  vertices = new float[mapsize * 6 * itemNums];
  
  nmSum(terrainVertexCalculation(vertices, map, mapsize));

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//glBufferStorage(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*mapsize * 6 * itemNums, vertices, GL_STATIC_DRAW);

	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, itemNums * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, itemNums * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, itemNums * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
    // normal map
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, itemNums * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);
    // tan
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, itemNums * sizeof(float), (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(4);
    // bitan
    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, itemNums * sizeof(float), (void*)(13 * sizeof(float)));
    glEnableVertexAttribArray(5);
    glBindVertexArray(0);

	//cube VAO
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube), &cube, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
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

	// Setup particalsVAO
	unsigned int particlesVBO, pariclesVAO;
	glGenVertexArrays(1, &pariclesVAO);
	glGenBuffers(1, &particlesVBO);
	glBindVertexArray(pariclesVAO);
	glBindBuffer(GL_ARRAY_BUFFER, particlesVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Load Textures
	// Cubemap (Skybox)
	vector<const GLchar*> faces;
	faces.push_back("skybox/sahara/right.tga");
	faces.push_back("skybox/sahara/left.tga");
	faces.push_back("skybox/sahara/top.tga");
	faces.push_back("skybox/sahara/bottom.tga");
	faces.push_back("skybox/sahara/back.tga");
	faces.push_back("skybox/sahara/front.tga");

  //faces.push_back("skybox/pure1/right.tga");
  //faces.push_back("skybox/pure1/back.tga");
  //faces.push_back("skybox/pure1/top.tga");
  //faces.push_back("skybox/pure1/bottom.tga");
  //faces.push_back("skybox/pure1/left.tga");
  //faces.push_back("skybox/pure1/front.tga");

  GLuint cubemapTexture = TextureLoading::LoadCubemap(faces);

	// load terrain texture
    terrainTexture = TextureLoading::LoadTexture((GLchar*)"texture/sand_texture3.jpg");
    // load normal map texture
    normalMap = TextureLoading::LoadTexture((GLchar*)"texture/sand_normal3.jpg");
	// load wood texture
    woodTexture = TextureLoading::LoadTexture((GLchar*)"texture/wood_texture.jpg");

	// depthmap
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
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_POLYGON_SMOOTH);
	// shader configuration
	// --------------------
	shader_shadow.use();
	shader_shadow.setInt("terrainTexture", 0);
	shader_shadow.setInt("depthMap", 1);
    shader_shadow.setInt("normalMap", 2);
	shader_cube.use();
	shader_cube.setInt("woodTexture", 0);
	shader_cube.setInt("depthMap", 1);
	/*shader.setInt("depthMap", 4);*/
	
	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	// particals init
	particals_operator test;

  deviations_x.push_back(24.1f); deviations_z.push_back(-0.9f); deviations_y.push_back(39.4f);
  deviations_x.push_back(35.4f); deviations_z.push_back(-1.2f); deviations_y.push_back(30.0f);
  deviations_x.push_back(28.0f); deviations_z.push_back(-1.6f); deviations_y.push_back(50.0f);
  deviations_x.push_back(22.1f); deviations_z.push_back(-1.7f); deviations_y.push_back(60.0f);
  deviations_x.push_back(23.7f); deviations_z.push_back(-1.45f); deviations_y.push_back(21.9f);
  deviations_x.push_back(54.3f); deviations_z.push_back(-1.41f); deviations_y.push_back(30.8f);
  deviations_x.push_back(53.7f); deviations_z.push_back(-1.45f); deviations_y.push_back(44.5f);
  deviations_x.push_back(40.5f); deviations_z.push_back(-3.2f); deviations_y.push_back(52.9f);
  deviations_x.push_back(33.4f); deviations_z.push_back(-1.91f); deviations_y.push_back(43.9f);
  deviations_x.push_back(45.3f); deviations_z.push_back(-2.8f); deviations_y.push_back(26.4f);

  deviations_x.push_back(30.2f); deviations_z.push_back(-2.7f); deviations_y.push_back(58.4f);
  deviations_x.push_back(20.0f); deviations_z.push_back(-1.3f); deviations_y.push_back(52.8f);
  deviations_x.push_back(17.6f); deviations_z.push_back(-2.17f); deviations_y.push_back(41.24f);
  deviations_x.push_back(28.4f); deviations_z.push_back(-1.6f); deviations_y.push_back(39.1f);
  deviations_x.push_back(29.8f); deviations_z.push_back(-1.75f); deviations_y.push_back(25.5f);
  deviations_x.push_back(29.86f); deviations_z.push_back(-2.02f); deviations_y.push_back(34.8f);
  deviations_x.push_back(36.8f); deviations_z.push_back(-2.25f); deviations_y.push_back(44.9f);
  deviations_x.push_back(47.67f); deviations_z.push_back(-2.85f); deviations_y.push_back(43.3f);
  deviations_x.push_back(40.2f); deviations_z.push_back(-3.19f); deviations_y.push_back(34.1f);
  deviations_x.push_back(51.37f); deviations_z.push_back(-2.15f); deviations_y.push_back(38.7f);

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

    //lightPos = camera.Position;

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		cubeModel = glm::mat4(1);
		model = glm::mat4(1);
		projection = glm::mat4(1);
		view = glm::mat4(1);
		skyboxModel = glm::mat4(1);

		projection = glm::perspective(camera.Zoom, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.01f, 100.0f);
		view = camera.GetViewMatrix();

		glStencilMask(0x00);

    	//scene rendered here
		glDepthFunc(GL_LESS);
	    DepthMap(shader_simple, ourModel);

		// Draw skybox at last
		glDepthFunc(GL_LEQUAL);  // Change depth function so depth test passes when values are equal to depth buffer's content
		skyboxShader.use();
    //skyboxModel = glm::rotate(skyboxModel, 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
		//skyboxModel = glm::translate(skyboxModel, glm::vec3(50.0f, 0.0f, -50.0f));
    skyboxModel = glm::translate(skyboxModel, glm::vec3(50.0f, -2.0f, 50.0f));
		skyboxModel = glm::scale(skyboxModel, glm::vec3(50.0f, 50.0f, 50.0f));

		//view = glm::mat4(glm::mat3(camera.GetViewMatrix()));	// Remove any translation component of the view modelmatrix
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

		glBindBuffer(GL_ARRAY_BUFFER, particlesVBO);
		glBindVertexArray(pariclesVAO);
		particalsShader.use();
		particalsShader.setMat4("model", model);
		particalsShader.setMat4("view", view);
		particalsShader.setMat4("projection", projection);
		test.init();
		test.update();
	  test.display(camera.Position.x, camera.Position.y, camera.Position.z);

		glBindVertexArray(0);

		// don't forget to enable shader before setting uniforms
		ourShader.use();
		// view/projection transformations
		//glm::mat4 treeProjection = glm::perspective(camera.Zoom, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
		//glm::mat4 treeView = camera.GetViewMatrix();
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);
		ourShader.setVec3("viewPos", camera.Position.x, camera.Position.y, camera.Position.z);
		// render the loaded model
		for (int i = 0; i < 20; i++) {
			glm::mat4 treeModel;
			treeModel = glm::translate(treeModel, glm::vec3(deviations_x[i], deviations_z[i], deviations_y[i]));
			treeModel = glm::scale(treeModel, glm::vec3(0.3f, 0.3f, 0.3f));	// it's a bit too big for our scene, so scale it down
			ourShader.setMat4("model", treeModel);
			ourModel.Draw(ourShader);
		}

		RenderText(textShader, "OpenGL is fun", 750.0f, 750.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f));

		//-------------------------------------------------------------------------------------------
		shader_shadow.use();

		shader_shadow.setMat4("view", view);
		shader_shadow.setMat4("projection", projection);
		shader_shadow.setVec3("lightPos", lightPos.x, lightPos.y, lightPos.z);
		shader_shadow.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
		shader_shadow.setVec3("viewPos", camera.Position.x, camera.Position.y, camera.Position.z);
		shader_shadow.setMat4("lightSpaceMatrix", lightSpaceMatrix);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, terrainTexture);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, normalMap);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		renderScene(shader_shadow);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//draw cube
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
		shader_cube.use();

		shader_cube.setMat4("view", view);
		shader_cube.setMat4("projection", projection);
		shader_cube.setVec3("lightPos", lightPos.x, lightPos.y, lightPos.z);
		shader_cube.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
		shader_cube.setVec3("viewPos", camera.Position.x, camera.Position.y, camera.Position.z);
		shader_cube.setMat4("lightSpaceMatrix", lightSpaceMatrix);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, woodTexture);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		renderCube(shader_cube);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//-------------------------------------------------------------------------------------------
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);
		glDisable(GL_DEPTH_TEST);

		shader_border.use();
		model = glm::translate(model, glm::vec3(41.0f, -2.0f, 41.0f));
		model = glm::rotate(model, 45.0f, glm::vec3(1.0, 1.0, 0.0));
		model = glm::scale(model, glm::vec3(2.2f, 2.2f, 2.2f));

		shader_border.setMat4("model", model);
		shader_border.setMat4("view", view);
		shader_border.setMat4("projection", projection);
		model = glm::mat4(1);

		glBindVertexArray(cubeVAO);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBindVertexArray(0);
		glStencilMask(0xFF);
		glEnable(GL_DEPTH_TEST);


		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	// release sources
	//ImGui_ImplGlfwGL3_Shutdown();
	//ImGui::DestroyContext();
  delete vertices;
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

vector<vector<glm::vec3>> terrainVertexCalculation(float *vertices, vector<vector<int>> map, int size) {
  vector<vector<glm::vec3>> nmArray;
  vector<glm::vec3> nmRow;
  int scaleSize = 15;
  for (int i = 0, j = 0, k = 0; i < size * 6 * itemNums - map[0].size() * 6 * itemNums; i += 6 * itemNums) {
    // vertex 1
    glm::vec3 vtx1pos = glm::vec3((float)j / (float)map[0].size(), (float)k / (float)map.size(), (float)map[j][k] / 256.0f / scaleSize);
    glm::vec2 vtx1txt = glm::vec2((float)j / (float)map[0].size() * 50, (float)k / (float)map.size() * 50);
    glm::vec2 vtx1nmmap = glm::vec2((float)j / (float)map[0].size() * 10, (float)k / (float)map.size() * 10);
    // vertex 2
    glm::vec3 vtx2pos = glm::vec3((float)j / (float)map[0].size(), (float)(k + 1) / (float)map.size(), (float)map[j][k + 1] / 256.0f / scaleSize);
    glm::vec2 vtx2txt = glm::vec2((float)j / (float)map[0].size() * 50, (float)(k + 1) / (float)map.size() * 50);
    glm::vec2 vtx2nmmap = glm::vec2((float)j / (float)map[0].size() * 10, (float)(k + 1) / (float)map.size() * 10);
    // vertex 3
    glm::vec3 vtx3pos = glm::vec3((float)(j + 1) / (float)map[0].size(), (float)k / (float)map.size(), (float)map[j + 1][k] / 256.0f / scaleSize);
    glm::vec2 vtx3txt = glm::vec2((float)(j + 1) / (float)map[0].size() * 50, (float)k / (float)map.size() * 50);
    glm::vec2 vtx3nmmap = glm::vec2((float)(j + 1) / (float)map[0].size() * 10, (float)k / (float)map.size() * 10);
    // vertex 4
    glm::vec3 vtx4pos = glm::vec3((float)(j + 1) / (float)map[0].size(), (float)(k + 1) / (float)map.size(), (float)map[j + 1][k + 1] / 256.0f / scaleSize);
    glm::vec2 vtx4txt = glm::vec2((float)(j + 1) / (float)map[0].size() * 50, (float)(k + 1) / (float)map.size() * 50);
    glm::vec2 vtx4nmmap = glm::vec2((float)(j + 1) / (float)map[0].size() * 10, (float)(k + 1) / (float)map.size() * 10);
    // triangle 1
    glm::vec3 tr1edge1 = vtx2pos - vtx1pos;
    glm::vec3 tr1edge2 = vtx3pos - vtx1pos;
    glm::vec2 tr1deltaUV1 = vtx2nmmap - vtx1nmmap;
    glm::vec2 tr1deltaUV2 = vtx3nmmap - vtx1nmmap;
    glm::vec3 tr1nm = crossProduct(tr1edge1, tr1edge2);
    glm::vec3 tr1tan(1);
    glm::vec3 tr1bitan(1);
    TBNcalculation(tr1edge1, tr1edge2, tr1deltaUV1, tr1deltaUV2, tr1tan, tr1bitan);
    // triangle 2
    glm::vec3 tr2edge1 = vtx2pos - vtx4pos;
    glm::vec3 tr2edge2 = vtx3pos - vtx4pos;
    glm::vec2 tr2deltaUV1 = vtx2nmmap - vtx4nmmap;
    glm::vec2 tr2deltaUV2 = vtx3nmmap - vtx4nmmap;
    glm::vec3 tr2nm = crossProduct(tr2edge1, tr2edge2);
    glm::vec3 tr2tan(1);
    glm::vec3 tr2bitan(1);
    TBNcalculation(tr2edge1, tr2edge2, tr2deltaUV1, tr2deltaUV2, tr2tan, tr2bitan);
    // add into array
    addVertice(vertices, i, vtx1pos, tr1nm, vtx1txt, vtx1nmmap, tr1tan, tr1bitan);
    addVertice(vertices, i + 1 * itemNums, vtx2pos, tr1nm, vtx2txt, vtx2nmmap, tr1tan, tr1bitan);
    addVertice(vertices, i + 2 * itemNums, vtx3pos, tr1nm, vtx3txt, vtx3nmmap, tr1tan, tr1bitan);
    addVertice(vertices, i + 3 * itemNums, vtx2pos, -tr2nm, vtx2txt, vtx2nmmap, tr2tan, tr2bitan);
    addVertice(vertices, i + 4 * itemNums, vtx3pos, -tr2nm, vtx3txt, vtx3nmmap, tr2tan, tr2bitan);
    addVertice(vertices, i + 5 * itemNums, vtx4pos, -tr2nm, vtx4txt, vtx4nmmap, tr2tan, tr2bitan);
    nmRow.push_back(tr1nm);
    nmRow.push_back(-tr2nm);
    k++;
    if (k >= (map[0].size() - 2)) {
      k = 0;
      j++;
      nmArray.push_back(nmRow);
      nmRow.clear();
    }
  }
  return nmArray;
}

void nmSum(vector<vector<glm::vec3>> nmArray) {
  for (int i = 1; i < nmArray.size() ; i++) {
    for (int j = 1; j < nmArray[0].size() - 1; j++) {
      glm::vec3 nm = glm::normalize(nmArray[i - 1][j - 1] + nmArray[i - 1][j] + nmArray[i - 1][j + 1] + nmArray[i][j - 1] + nmArray[i][j] + nmArray[i][j + 1]);
      // i-1 j-1
      vertices[(i - 1) * itemNums * nmArray[0].size() * 3 + 3 * itemNums * (j - 1) + 2 * itemNums + 3] = nm.x;
      vertices[(i - 1) * itemNums * nmArray[0].size() * 3 + 3 * itemNums * (j - 1) + 2 * itemNums + 4] = nm.y;
      vertices[(i - 1) * itemNums * nmArray[0].size() * 3 + 3 * itemNums * (j - 1) + 2 * itemNums + 5] = nm.z;
      // i-1 j
      vertices[(i - 1) * itemNums * nmArray[0].size() * 3 + 3 * itemNums * j + 2 * itemNums + 3] = nm.x;
      vertices[(i - 1) * itemNums * nmArray[0].size() * 3 + 3 * itemNums * j + 2 * itemNums + 4] = nm.y;
      vertices[(i - 1) * itemNums * nmArray[0].size() * 3 + 3 * itemNums * j + 2 * itemNums + 5] = nm.z;
      // i-1 j+1
      vertices[(i - 1) * itemNums * nmArray[0].size() * 3 + 3 * itemNums * (j + 1) + 1 * itemNums + 3] = nm.x;
      vertices[(i - 1) * itemNums * nmArray[0].size() * 3 + 3 * itemNums * (j + 1) + 1 * itemNums + 4] = nm.y;
      vertices[(i - 1) * itemNums * nmArray[0].size() * 3 + 3 * itemNums * (j + 1) + 1 * itemNums + 5] = nm.z;
      // i j-1
      vertices[i * itemNums * nmArray[0].size() * 3 + 3 * itemNums * (j - 1) + 1 * itemNums + 3] = nm.x;
      vertices[i * itemNums * nmArray[0].size() * 3 + 3 * itemNums * (j - 1) + 1 * itemNums + 4] = nm.y;
      vertices[i * itemNums * nmArray[0].size() * 3 + 3 * itemNums * (j - 1) + 1 * itemNums + 5] = nm.z;
      // i j
      vertices[i * itemNums * nmArray[0].size() * 3 + 3 * itemNums * j + 3] = nm.x;
      vertices[i * itemNums * nmArray[0].size() * 3 + 3 * itemNums * j + 4] = nm.y;
      vertices[i * itemNums * nmArray[0].size() * 3 + 3 * itemNums * j + 5] = nm.z;
      // i j+1
      vertices[i * itemNums * nmArray[0].size() * 3 + 3 * itemNums * (j + 1) + 3] = nm.x;
      vertices[i * itemNums * nmArray[0].size() * 3 + 3 * itemNums * (j + 1) + 4] = nm.y;
      vertices[i * itemNums * nmArray[0].size() * 3 + 3 * itemNums * (j + 1) + 5] = nm.z;
    }
  }
}

//vector<glm::vec3> normalCalculation(glm::vec3 nm1, glm::vec3 nm2, glm::vec3 nm3, glm::vec3 nm4, glm::vec3 nm5, glm::vec3 nm6, int row, int col) {
//  vector<glm::vec3> verticeNm;
//  for (int i = col * itemNums, j = 1, k = 0; i < size * 6 * itemNums - map[0].size() * 6 * itemNums; i += itemNums) {
//
//  }
//}



void TBNcalculation(glm::vec3 edge1, glm::vec3 edge2, glm::vec2 deltaUV1, glm::vec2 deltaUV2, glm::vec3 &tan, glm::vec3 &bitan) {
  GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

  tan.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
  tan.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
  tan.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
  tan = glm::normalize(tan);

  bitan.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
  bitan.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
  bitan.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
  bitan = glm::normalize(bitan);
}

void renderScene(Shader &shader1) {
	// Draw terrain.
	shader1.use();
	model = glm::scale(model, glm::vec3(100.0f, 100.0f, 100.0f));
	model = glm::rotate(model, 90.0f, glm::vec3(1.0, 0.0, 0.0));
	shader1.setMat4("model", model);
	model = glm::mat4(1);
	glBindVertexArray(VAO);
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glDrawArrays(GL_TRIANGLES, 0, mapsize * 6);
  //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindVertexArray(0);
}

void renderModel(Shader &shader1, Model &ourModel) {
  shader1.use();
  //ourShader.setVec3("viewPos", camera.Position.x, camera.Position.y, camera.Position.z);
  // render the loaded model
  for (int i = 0; i < 20; i++) {
    treeModel = glm::mat4(1);
    treeModel = glm::translate(treeModel, glm::vec3(deviations_x[i], deviations_z[i], deviations_y[i]));
    treeModel = glm::scale(treeModel, glm::vec3(0.3f, 0.3f, 0.3f));	// it's a bit too big for our scene, so scale it down
    shader1.setMat4("model", treeModel);
    ourModel.Draw(shader1);
  }
}

void renderCube(Shader &shader1) {
	shader1.use();
	
	model = glm::translate(model, glm::vec3(41.0f, -2.0f, 41.0f));
	model = glm::rotate(model, 45.0f, glm::vec3(1.0, 1.0, 0.0));
	model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
	
	shader1.setMat4("model", model);
	model = glm::mat4(1);
	glBindVertexArray(cubeVAO);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindVertexArray(0);
}

void DepthMap(Shader &shader1, Model &ourModel) {
	model = glm::scale(model, glm::vec3(100.0f, 100.0f, 100.0f));
	model = glm::rotate(model, 90.0f, glm::vec3(1.0, 0.0, 0.0));

	GLfloat near_plane =1.0f, far_plane = 100.0f;
  	glm::mat4 lightProjection = glm::perspective(45.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.01f, 500.0f);
    glm::mat4 lightView = glm::lookAt(
      lightPos,
      glm::vec3(48.0f, 0.0f, 40.0f),
      glm::vec3(0.0f, 1.0f, 0.0f)
    );

    //glm::mat4 lightProjection = glm::perspective(camera.Zoom, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.01f, 100.0f);
    //glm::mat4 lightView = camera.GetViewMatrix();

	lightSpaceMatrix = lightProjection * lightView;
	
	shader1.use();
	shader1.setMat4("lightSpaceMatrix", lightSpaceMatrix);

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);
    renderScene(shader1);
    renderModel(shader1, ourModel);
    glCullFace(GL_BACK);
	renderCube(shader1);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
}

void addVertice(float *vertices, int offset, glm::vec3 pos, glm::vec3 nm, glm::vec2 txt, glm::vec2 nmmap, glm::vec3 tan, glm::vec3 bitan) {
  vertices[offset] = pos.x;
  vertices[offset + 1] = pos.y;
  vertices[offset + 2] = pos.z;
  vertices[offset + 3] = nm.x;
  vertices[offset + 4] = nm.y;
  vertices[offset + 5] = nm.z;
  vertices[offset + 6] = txt.x;
  vertices[offset + 7] = txt.y;
  vertices[offset + 8] = nmmap.x;
  vertices[offset + 9] = nmmap.y;
  vertices[offset + 10] = tan.x;
  vertices[offset + 11] = tan.y;
  vertices[offset + 12] = tan.z;
  vertices[offset + 13] = bitan.x;
  vertices[offset + 14] = bitan.y;
  vertices[offset + 15] = bitan.z;
}

void RenderText(Shader &shader, string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
	// Activate corresponding render state	
	shader.use();
	shader.setVec3("textColor", color.x, color.y, color.z);
	//glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(TextVAO);

	// Iterate through all characters
	string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = Characters[*c];

		GLfloat xpos = x + ch.Bearing.x * scale;
		GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		GLfloat w = ch.Size.x * scale;
		GLfloat h = ch.Size.y * scale;
		// Update VBO for each character
		GLfloat vertices[6][4] = {
			{ xpos,     ypos + h,   0.0, 0.0 },
		{ xpos,     ypos,       0.0, 1.0 },
		{ xpos + w, ypos,       1.0, 1.0 },

		{ xpos,     ypos + h,   0.0, 0.0 },
		{ xpos + w, ypos,       1.0, 1.0 },
		{ xpos + w, ypos + h,   1.0, 0.0 }
		};
		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, TextVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}


/*
// calculate
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

//
glm::vec3 Nor = n1 + n2 + n3 + n4;

vertices[i + 3] = Nor.x;
vertices[i + 4] = Nor.y;
vertices[i + 5] = Nor.z;
*/