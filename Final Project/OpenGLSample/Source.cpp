#include "ShapeGenerator.h"
#include "ShapeData.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "shader.h"
#include "camera.h"

#include "cylinder.h"


#include <iostream>



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);

void MyProcessMouseScroll(float yoffset);

struct GLShape
{
	GLuint vao;
	GLuint vbo;
	GLuint Vertices;
};

struct GLTorus
{
	GLuint vao;
	GLuint vbo;
	GLuint uvbo;
	GLuint Vertices;
};

void CreateRectangle(GLShape& shape);
void CreateCubeNoTop(GLShape& shape);
void CreatePyramid(GLShape& shape);
void CreateOpenPyramid(GLShape& shape);
void CreateTorus(GLTorus& torus);

void setCoords(double r, double c, int rSeg, int cSeg, int i, int j, GLfloat* vertices, GLfloat* uv);
int createObject(double r, double c, int rSeg, int cSeg, GLfloat** vertices, GLfloat** uv);


// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Shapes
GLShape gCenterCube;
GLShape gLeftCube;
GLShape gRightCube;
GLShape gRectangle;
GLShape gPlane;
GLShape gBottomPyramid;
GLShape gTopOpenPyramid;
GLTorus gTorus;

// plane
GLuint planeNumIndices;
GLuint planeIndexByteOffset;

// sphere
GLuint sphereNumIndices;
GLuint sphereIndexByteOffset;

// camera
Camera camera(glm::vec3(1.5f, 3.0f, 6.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

bool perspective = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float cameraSpeed = 2.5f;

// offset variables for plane, sphere
const uint NUM_VERTICES_PER_TRI = 3;
const uint NUM_FLOATS_PER_VERTICE = 9;
const uint VERTEX_BYTE_SIZE = NUM_FLOATS_PER_VERTICE * sizeof(float);

// projection matrix
glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Final Project", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile our shader zprogram
	// ------------------------------------

	Shader lightingShader("shaderfiles/6.multiple_lights.vs", "shaderfiles/6.multiple_lights.fs");
	Shader lightCubeShader("shaderfiles/6.light_cube.vs", "shaderfiles/6.light_cube.fs");
	Shader torusShader("shaderfiles/TransformVertexShader.vertexshader", "shaderfiles/TextureFragmentShader.fragmentshader");

	// positions of the point lights
	glm::vec3 pointLightPositions[] = {
		glm::vec3(0.8f,  2.8f,  2.0f),
		glm::vec3(0.0f, -5.0f, 2.0f),
		glm::vec3(-4.2f,  0.0f, 2.0f),
		glm::vec3(5.0f,  1.0f, 3.0f)
	};

	// set up vertex data (and buffer(s)) and configure vertex attributes
// ------------------------------------------------------------------
	float cylinderVertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};

	unsigned int cylinderVBO, cylinderVAO;
	//unsigned int VBO2, VAO2;

	glGenVertexArrays(1, &cylinderVAO);
	glGenBuffers(1, &cylinderVBO);
	glBindVertexArray(cylinderVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cylinderVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cylinderVertices), cylinderVertices, GL_STATIC_DRAW);


	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// cup handle cube's VAO and VBO
	unsigned int cubeVBO, cubeVAO;

	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);

	glBindVertexArray(cubeVAO);
	// position attribute for cube
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture attribute for cube
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// normal attribute for cube
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);


	// position attribute for cube
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture attribute for cube
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// normal attribute for cube
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// lightCube's VAO and VBO
	unsigned int lightCubeVAO;
	glGenVertexArrays(1, &lightCubeVAO);
	glBindVertexArray(lightCubeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	// note that we update the lamp's position attribute's stride to reflect the updated buffer data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Create cubes
	CreateCubeNoTop(gLeftCube);
	CreateCubeNoTop(gRightCube);
	CreateCubeNoTop(gCenterCube);
	
	// Create rectangle
	CreateRectangle(gRectangle);

	// Create pyramid - bottom of glass
	CreatePyramid(gBottomPyramid);

	// Create open pyramid - top of glass
	CreateOpenPyramid(gTopOpenPyramid);

	// Create torus
	CreateTorus(gTorus);

// creates plane object
	ShapeData plane = ShapeGenerator::makePlane(30);

	unsigned int planeVBO{}, planeVAO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);

	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, plane.vertexBufferSize() + plane.indexBufferSize(), 0, GL_STATIC_DRAW);

	GLsizeiptr currentOffset = 0;

	glBufferSubData(GL_ARRAY_BUFFER, currentOffset, plane.vertexBufferSize(), plane.vertices);
	currentOffset += plane.vertexBufferSize();

	planeIndexByteOffset = currentOffset;

	glBufferSubData(GL_ARRAY_BUFFER, currentOffset, plane.indexBufferSize(), plane.indices);

	planeNumIndices = plane.numIndices;

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, VERTEX_BYTE_SIZE, (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, VERTEX_BYTE_SIZE, (void*)(sizeof(float) * 3));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, VERTEX_BYTE_SIZE, (void*)(sizeof(float) * 6));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeVBO);

// creates sphere object
	ShapeData sphere = ShapeGenerator::makeSphere();

	unsigned int sphereVBO{}, sphereVAO;
	glGenVertexArrays(1, &sphereVAO);
	glGenBuffers(1, &sphereVBO);
	glBindVertexArray(sphereVAO);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	glBufferData(GL_ARRAY_BUFFER, sphere.vertexBufferSize() + sphere.indexBufferSize(), 0, GL_STATIC_DRAW);
	currentOffset = 0;
	glBufferSubData(GL_ARRAY_BUFFER, currentOffset, sphere.vertexBufferSize(), sphere.vertices);
	currentOffset += sphere.vertexBufferSize();
	sphereIndexByteOffset = currentOffset;
	glBufferSubData(GL_ARRAY_BUFFER, currentOffset, sphere.indexBufferSize(), sphere.indices);
	sphereNumIndices = sphere.numIndices;
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, VERTEX_BYTE_SIZE, (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, VERTEX_BYTE_SIZE, (void*)(sizeof(float) * 3));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, VERTEX_BYTE_SIZE, (void*)(sizeof(float) * 6));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereVBO);

	// load textures using utility function
	unsigned int marbleMap = loadTexture("images/marble.jpg");
	unsigned int woodMap = loadTexture("images/new-wood.jpg");
	unsigned int woodGrainMap = loadTexture("images/Wood-grain.jpg");
	unsigned int greenSwirl = loadTexture("images/green_swirl.jpg");
	unsigned int blackTextureMap = loadTexture("images/container2_specular.jpg");

	// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	// -------------------------------------------------------------------------------------------
	lightingShader.use();
	lightingShader.setInt("material.diffuse", 0);
	lightingShader.setInt("material.specular", 1);



	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		lightingShader.use();
		lightingShader.setVec3("viewPos", camera.Position);
		lightingShader.setFloat("material.shininess", 32.0f);

		// directional light
		lightingShader.setVec3("dirLight.direction", 2.5f, 0.0f, 0.0f);
		lightingShader.setVec3("dirLight.ambient", 0.10f, 0.10f, 0.10f);
		lightingShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
		lightingShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
		// key light
		lightingShader.setVec3("pointLights[0].position", pointLightPositions[0]);
		lightingShader.setVec3("pointLights[0].ambient", 0.5f, 0.5f, 0.5f);
		lightingShader.setVec3("pointLights[0].diffuse", 0.5f, 0.5f, 0.5f);
		lightingShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
		lightingShader.setFloat("pointLights[0].constant", 1.0f);
		lightingShader.setFloat("pointLights[0].linear", 0.09);
		lightingShader.setFloat("pointLights[0].quadratic", 0.032);
		// fill light
		lightingShader.setVec3("pointLights[1].position", pointLightPositions[1]);
		lightingShader.setVec3("pointLights[1].ambient", 0.7f, 0.7f, 0.7f);
		lightingShader.setVec3("pointLights[1].diffuse", 0.1f, 0.1f, 0.1f);
		lightingShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
		lightingShader.setFloat("pointLights[1].constant", 1.0f);
		lightingShader.setFloat("pointLights[1].linear", 0.09);
		lightingShader.setFloat("pointLights[1].quadratic", 0.032);
		// point light 3
		lightingShader.setVec3("pointLights[2].position", pointLightPositions[2]);
		lightingShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
		lightingShader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
		lightingShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
		lightingShader.setFloat("pointLights[2].constant", 1.0f);
		lightingShader.setFloat("pointLights[2].linear", 0.09);
		lightingShader.setFloat("pointLights[2].quadratic", 0.032);
		// point light 4
		lightingShader.setVec3("pointLights[3].position", pointLightPositions[3]);
		lightingShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
		lightingShader.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
		lightingShader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
		lightingShader.setFloat("pointLights[3].constant", 1.0f);
		lightingShader.setFloat("pointLights[3].linear", 0.09);
		lightingShader.setFloat("pointLights[3].quadratic", 0.032);
		// spotLight
		lightingShader.setVec3("spotLight.position", camera.Position);
		lightingShader.setVec3("spotLight.direction", camera.Front);
		lightingShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
		lightingShader.setVec3("spotLight.diffuse", 0.7f, 0.7f, 0.7f);
		lightingShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
		lightingShader.setFloat("spotLight.constant", 1.0f);
		lightingShader.setFloat("spotLight.linear", 0.09);
		lightingShader.setFloat("spotLight.quadratic", 0.032);
		lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
		lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

		// view/projection transformations
		glm::mat4 view = camera.GetViewMatrix();
		lightingShader.setMat4("projection", projection);
		lightingShader.setMat4("view", view);

		// world transformation
		glm::mat4 model = glm::mat4(1.0f);
		lightingShader.setMat4("model", model);

		glm::mat4 scale;
		glm::mat4 rotation;
		glm::mat4 translation;
		glm::mat4 projection;
		GLint modelLoc;
		GLint viewLoc;
		GLint projLoc;

// setup to draw plane
		glBindTexture(GL_TEXTURE_2D, woodMap);
		glBindVertexArray(planeVAO);
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(70.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		lightingShader.setMat4("model", model);

		// draw plane
		glDrawElements(GL_TRIANGLES, planeNumIndices, GL_UNSIGNED_SHORT, (void*)planeIndexByteOffset);

// rectangle

		model = model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		model = glm::translate(model, glm::vec3(0.0f, 4.5f, 0.0f));
		model = glm::rotate(model, glm::radians(70.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		lightingShader.setMat4("model", model);

		// Set the shader to be used
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, woodGrainMap);

		// Activate the VBOs contained within the mesh's VAO
		glBindVertexArray(gRectangle.vao);

		// Draws the rectangle
		glDrawArrays(GL_TRIANGLES, 0, gRectangle.Vertices);

		glClear(GL_DEPTH_BUFFER_BIT);
		
// left cube
		// Model Matrix - steps 1-4
		model = model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(1.2f, 1.2f, 1.2f));
		model = glm::translate(model, glm::vec3(-2.2f, 3.9f, 0.0f));
		model = glm::rotate(model, glm::radians(70.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		// Set the shader to be used
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, woodMap);

		// Activate the VBOs contained within the mesh's VAO
		glBindVertexArray(gLeftCube.vao);

		lightingShader.setMat4("model", model);

		// Draws the left cube
		glDrawArrays(GL_TRIANGLES, 0, gLeftCube.Vertices);

// middle cube
	   // Model Matrix - steps 1-4
		model = model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(1.2f, 1.2f, 1.2f));
		model = glm::translate(model, glm::vec3(0.1f, 3.9f, 0.0f));
		model = glm::rotate(model, glm::radians(70.0f), glm::vec3(1.0f, 0.0f, 0.0f)); //90.0 1 0 0 makes it sit flat, 120.0 makes it tilt forward

		// Set the shader to be used
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, woodMap);

		// Activate the VBOs contained within the mesh's VAO
		glBindVertexArray(gCenterCube.vao);

		lightingShader.setMat4("model", model);

		// Draws the middle cube
		glDrawArrays(GL_TRIANGLES, 0, gCenterCube.Vertices);

// right cube
		// Model Matrix - steps 1-4
		model = model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(1.2f, 1.2f, 1.2f));
		model = glm::translate(model, glm::vec3(2.2f, 3.9f, 0.0f));
		model = glm::rotate(model, glm::radians(70.0f), glm::vec3(1.0f, 0.0f, 0.0f)); //90.0 1 0 0 makes it sit flat, 120.0 makes it tilt forward

		// Set the shader to be used
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, woodMap);

		// Activate the VBOs contained within the mesh's VAO
		glBindVertexArray(gRightCube.vao);

		lightingShader.setMat4("model", model);

		// Draws the right cube
		glDrawArrays(GL_TRIANGLES, 0, gRightCube.Vertices);


// setup to draw sphere
		glBindTexture(GL_TEXTURE_2D, marbleMap);
		glBindVertexArray(sphereVAO);
		model = model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(70.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, glm::vec3(-5.2f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.5f));
		lightingShader.setMat4("model", model);

		// draw sphere
		glDrawElements(GL_TRIANGLES, sphereNumIndices, GL_UNSIGNED_SHORT, (void*)sphereIndexByteOffset);
		
// cylinder - head
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, blackTextureMap);
		glBindVertexArray(cylinderVAO);
		model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first	
		model = glm::rotate(model, glm::radians(70.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 5.0f));
		model = glm::scale(model, glm::vec3(0.9f));
		lightingShader.setMat4("model", model);


		static_meshes_3D::Cylinder C(2, 30, .3, true, true, true);
		C.render();

// cylinder - left ear
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, blackTextureMap);
		glBindVertexArray(cylinderVAO);
		model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first	
		model = glm::rotate(model, glm::radians(70.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, glm::vec3(-2.5f, 0.0f, 3.0f));
		//model = glm::scale(model, glm::vec3(0.5f));
		lightingShader.setMat4("model", model);


		static_meshes_3D::Cylinder Cl(1, 30, .3, true, true, true);
		Cl.render();

// cylinder - right ear
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, blackTextureMap);
		glBindVertexArray(cylinderVAO);
		model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first	
		model = glm::rotate(model, glm::radians(70.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, glm::vec3(0.6f, 0.0f, 3.0f));
		lightingShader.setMat4("model", model);


		static_meshes_3D::Cylinder Cr(1, 30, .3, true, true, true);
		Cr.render();

// Cylinder - Base of glass

		// cylinder - right ear
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, greenSwirl);
		glBindVertexArray(cylinderVAO);
		model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first	
		model = glm::rotate(model, glm::radians(70.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, glm::vec3(5.0f, 0.0f, 0.0f));
		lightingShader.setMat4("model", model);


		static_meshes_3D::Cylinder CBase(0.8, 30, 0.1, true, true, true);
		CBase.render();

// Pyramid - bottom glass
		model = model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		model = glm::translate(model, glm::vec3(5.1f, 0.3f, 0.5f));
		model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.5f, 1.0f, 0.0f));
		lightingShader.setMat4("model", model);

		// Set the shader to be used
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, greenSwirl);

		// Activate the VBOs contained within the mesh's VAO
		glBindVertexArray(gBottomPyramid.vao);

		// Draws the rectangle
		glDrawArrays(GL_TRIANGLES, 0, gBottomPyramid.Vertices);

// cylinder - stem of glass 
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, greenSwirl);
		glBindVertexArray(cylinderVAO);
		model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first	
		model = glm::rotate(model, glm::radians(70.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, glm::vec3(5.0f, 1.0f, 0.0f));
		lightingShader.setMat4("model", model);


		static_meshes_3D::Cylinder CStem(0.2, 30, 2, true, true, true);
		CStem.render();

// Open Pyramid - top of glass
		model = model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
		model = glm::translate(model, glm::vec3(2.5f, 0.3f, 0.85f));
		model = glm::rotate(model, glm::radians(260.0f), glm::vec3(0.5f, 1.0f, 0.0f));
		lightingShader.setMat4("model", model);

		// Set the shader to be used
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, greenSwirl);

		// Activate the VBOs contained within the mesh's VAO
		glBindVertexArray(gTopOpenPyramid.vao);

		// Draws the pyramid
		glDrawArrays(GL_TRIANGLES, 0, gTopOpenPyramid.Vertices);

		glClear(GL_DEPTH_BUFFER_BIT);

// Torus
		model = model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
		model = glm::translate(model, glm::vec3(25.0f, 5.0f, 13.0f));
		model = glm::rotate(model, glm::radians(150.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		lightingShader.setMat4("model", model);

		// Set the shader to be used
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, greenSwirl);

		// Activate the VBOs contained within the mesh's VAO
		glBindVertexArray(gTorus.vao);

		// Draws the torus
		glDrawArrays(GL_TRIANGLES, 0, gTorus.Vertices);

		glClear(GL_DEPTH_BUFFER_BIT);
		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteBuffers(1, &planeVBO);
	glDeleteVertexArrays(1, &gRectangle.vao);
	glDeleteBuffers(1, &gRectangle.vbo);
	glDeleteVertexArrays(1, &gCenterCube.vao);
	glDeleteBuffers(1, &gCenterCube.vbo);
	glDeleteVertexArrays(1, &gLeftCube.vao);
	glDeleteBuffers(1, &gLeftCube.vbo);
	glDeleteVertexArrays(1, &gRightCube.vao);
	glDeleteBuffers(1, &gRightCube.vbo);
	glDeleteVertexArrays(1, &sphereVAO);
	glDeleteBuffers(1, &sphereVBO);
	glDeleteVertexArrays(1, &gBottomPyramid.vao);
	glDeleteBuffers(1, &gBottomPyramid.vbo);
	glDeleteVertexArrays(1, &gTopOpenPyramid.vao);
	glDeleteBuffers(1, &gTopOpenPyramid.vbo);
	glDeleteVertexArrays(1, &gTorus.vao);
	glDeleteBuffers(1, &gTorus.vbo);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float cameraOffset = cameraSpeed * deltaTime;
	
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);

	// change view between perspective and orthographics
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
		if (perspective) {
			projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
			perspective = false;
		}
		else {
			projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
			perspective = true;
		}
	}

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	MyProcessMouseScroll(yoffset);
}

unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

void MyProcessMouseScroll(float yoffset)
{
	cameraSpeed += (float)yoffset;

	cameraSpeed = std::fmaxf(cameraSpeed, 1);
	cameraSpeed = std::fminf(cameraSpeed, 100);
}

void CreateRectangle(GLShape& shape)
{
	//Vertex data
	GLfloat verts[] = {
		// Vertex Positions    // Normals         // Texture

		//Top
		-4.0f, +1.5f, +1.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
		+4.0f, +1.5f, +1.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
		+4.0f, +1.5f, -1.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f,
		-4.0f, +1.5f, +1.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
		+4.0f, +1.5f, -1.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f,
		-4.0f, +1.5f, -1.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,

		//Front
		-4.0f, +1.5f, -1.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
		+4.0f, +1.5f, -1.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,
		+4.0f, -1.5f, -1.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
		-4.0f, +1.5f, -1.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
		+4.0f, -1.5f, -1.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
		-4.0f, -1.5f, -1.5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,

		//Right
		+4.0f, +1.5f, -1.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
		+4.0f, +1.5f, +1.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
		+4.0f, -1.5f, +1.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
		+4.0f, +1.5f, -1.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
		+4.0f, -1.5f, +1.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
		+4.0f, -1.5f, -1.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,

		//Left
		-4.0f, +1.5f, +1.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
		-4.0f, +1.5f, -1.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
		-4.0f, -1.5f, -1.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
		-4.0f, +1.5f, +1.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
		-4.0f, -1.5f, -1.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
		-4.0f, -1.5f, +1.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,

		//Back
		+4.0f, +1.5f, +1.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
		-4.0f, +1.5f, +1.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
		-4.0f, -1.5f, +1.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
		+4.0f, +1.5f, +1.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
		-4.0f, -1.5f, +1.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
		+4.0f, -1.5f, +1.5f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f,

		//Bottom
		+4.0f, -1.5f, -1.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
		-4.0f, -1.5f, +1.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
		-4.0f, -1.5f, -1.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
		+4.0f, -1.5f, -1.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
		+4.0f, -1.5f, +1.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
		-4.0f, -1.5f, +1.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
	};

	// Creates the Vertex Attribute Pointer
	const GLuint floatsPerVertex = 3; // Number of coordinates per vertex
	const GLuint floatsPerNormal = 3; // (r,g,b,a)
	const GLuint floatsPerTexture = 2; // Texture

	shape.Vertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerTexture));

	glGenVertexArrays(1, &shape.vao);
	glGenBuffers(1, &shape.vbo);

	glBindVertexArray(shape.vao);

	glBindBuffer(GL_ARRAY_BUFFER, shape.vbo);  // Activates the buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);  // Sends vertex or coordinate data to the GPU

	// Strides between vertex coordinates is 6 (x, y, r, g, b, a). A tightly packed stride is 0.
	GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerTexture); // number of floats before each

	// pointer for the vertices
	glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);  // Position of coordinates in the buffer

	// pointer for the colors
	glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
	glEnableVertexAttribArray(1); // Position of color data in buffer

	// pointer for the texture
	glVertexAttribPointer(2, floatsPerTexture, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
	glEnableVertexAttribArray(2); // Position of texture data in buffer

}

void CreateCubeNoTop(GLShape& shape)
{
	//Vertex data
	GLfloat verts[] = {
		// Vertex Positions    // Colors            // Texture

		// Front
		-1.0f, +1.0f, -1.0f, +0.40f, +0.26f, +0.13f, +1.0f, 1.0f, 1.0f,
		+1.0f, +1.0f, -1.0f, +0.40f, +0.26f, +0.13f, +1.0f, 0.0f, 1.0f,
		+1.0f, -1.0f, -1.0f, +0.40f, +0.26f, +0.13f, +1.0f, 0.0f, 0.0f,
		-1.0f, +1.0f, -1.0f, +0.40f, +0.26f, +0.13f, +1.0f, 1.0f, 1.0f,
		+1.0f, -1.0f, -1.0f, +0.40f, +0.26f, +0.13f, +1.0f, 0.0f, 0.0f,
		-1.0f, -1.0f, -1.0f, +0.40f, +0.26f, +0.13f, +1.0f, 1.0f, 0.0f,

		// Right
		+1.0f, +1.0f, -1.0f, +0.40f, +0.26f, +0.13f, +1.0f, 1.0f, 1.0f,
		+1.0f, +1.0f, +1.0f, +0.40f, +0.26f, +0.13f, +1.0f, 0.0f, 1.0f,
		+1.0f, -1.0f, +1.0f, +0.40f, +0.26f, +0.13f, +1.0f, 0.0f, 0.0f,
		+1.0f, +1.0f, -1.0f, +0.40f, +0.26f, +0.13f, +1.0f, 1.0f, 1.0f,
		+1.0f, -1.0f, +1.0f, +0.40f, +0.26f, +0.13f, +1.0f, 0.0f, 0.0f,
		+1.0f, -1.0f, -1.0f, +0.40f, +0.26f, +0.13f, +1.0f, 1.0f, 0.0f,

		// Left
		-1.0f, +1.0f, +1.0f, +0.40f, +0.26f, +0.13f, +1.0f, 1.0f, 1.0f,
		-1.0f, +1.0f, -1.0f, +0.40f, +0.26f, +0.13f, +1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, -1.0f, +0.40f, +0.26f, +0.13f, +1.0f, 0.0f, 0.0f,
		-1.0f, +1.0f, +1.0f, +0.40f, +0.26f, +0.13f, +1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f, +0.40f, +0.26f, +0.13f, +1.0f, 0.0f, 0.0f,
		-1.0f, -1.0f, +1.0f, +0.40f, +0.26f, +0.13f, +1.0f, 1.0f, 0.0f,

		// Back
		+1.0f, +1.0f, +1.0f, +0.40f, +0.26f, +0.13f, +1.0f, 1.0f, 1.0f,
		-1.0f, +1.0f, +1.0f, +0.40f, +0.26f, +0.13f, +1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, +1.0f, +0.40f, +0.26f, +0.13f, +1.0f, 0.0f, 0.0f,
		+1.0f, +1.0f, +1.0f, +0.40f, +0.26f, +0.13f, +1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, +1.0f, +0.40f, +0.26f, +0.13f, +1.0f, 0.0f, 0.0f,
		+1.0f, -1.0f, +1.0f, +0.40f, +0.26f, +0.13f, +1.0f, 1.0f, 0.0f,

		// Bottom
		+1.0f, -1.0f, -1.0f, +0.40f, +0.26f, +0.13f, +1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f, +0.40f, +0.26f, +0.13f, +1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, +1.0f, +0.40f, +0.26f, +0.13f, +1.0f, 0.0f, 0.0f,
		+1.0f, -1.0f, -1.0f, +0.40f, +0.26f, +0.13f, +1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, +1.0f, +0.40f, +0.26f, +0.13f, +1.0f, 0.0f, 0.0f,
		+1.0f, -1.0f, +1.0f, +0.40f, +0.26f, +0.13f, +1.0f, 1.0f, 0.0f,

	};

	// Creates the Vertex Attribute Pointer
	const GLuint floatsPerVertex = 3; // Number of coordinates per vertex
	const GLuint floatsPerColor = 4; // (r,g,b,a)
	const GLuint floatsPerTexture = 2; // Texture

	shape.Vertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerColor + floatsPerTexture));

	glGenVertexArrays(1, &shape.vao);
	glGenBuffers(1, &shape.vbo);

	glBindVertexArray(shape.vao);

	glBindBuffer(GL_ARRAY_BUFFER, shape.vbo);  // Activates the buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);  // Sends vertex or coordinate data to the GPU

	// Strides between vertex coordinates is 6 (x, y, r, g, b, a). A tightly packed stride is 0.
	GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor + floatsPerTexture); // number of floats before each

	// pointer for the vertices
	glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);  // Position of coordinates in the buffer

	// pointer for the colors
	//glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
	glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
	glEnableVertexAttribArray(1); // Position of color data in buffer

	// pointer for the texture
	glVertexAttribPointer(2, floatsPerTexture, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerColor)));
	glEnableVertexAttribArray(2); // Position of texture data in buffer

}

void CreatePyramid(GLShape& shape)
{
	//Vertex data
	GLfloat verts[] = {
		// Vertex Positions    // Colors            // Texture

		// Far side
		 0.0f,  0.0f,  0.0f, +0.40f, +0.26f, +0.13f, +1.0f, 0.5f, 1.0f,
		-0.5f,  0.5f, -0.5f, +0.40f, +0.26f, +0.13f, +1.0f, 1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, +0.40f, +0.26f, +0.13f, +1.0f, 0.0f, 0.0f,

		// Near side
		 0.0f,  0.0f,  0.0f, +0.40f, +0.26f, +0.13f, +1.0f, 0.5f, 1.0f,
		-0.5f,  0.5f,  0.5f, +0.40f, +0.26f, +0.13f, +1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f,  0.5f, +0.40f, +0.26f, +0.13f, +1.0f, 1.0f, 0.0f,

		// Bottom
		-0.5f,  0.5f,  0.5f, +0.40f, +0.26f, +0.13f, +1.0f, 1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, +0.40f, +0.26f, +0.13f, +1.0f, 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, +0.40f, +0.26f, +0.13f, +1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, +0.40f, +0.26f, +0.13f, +1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f,  0.5f, +0.40f, +0.26f, +0.13f, +1.0f, 1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f, +0.40f, +0.26f, +0.13f, +1.0f, 1.0f, 0.0f,

		// Left side
		 0.0f,  0.0f,  0.0f, +0.40f, +0.26f, +0.13f, +1.0f, 0.5f, 1.0f,
		-0.5f, -0.5f,  0.5f, +0.40f, +0.26f, +0.13f, +1.0f, 1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, +0.40f, +0.26f, +0.13f, +1.0f, 0.0f, 0.0f,

		// Right side
		 0.0f,  0.0f,  0.0f, +0.40f, +0.26f, +0.13f, +1.0f, 0.5f, 1.0f,
		-0.5f,  0.5f,  0.5f, +0.40f, +0.26f, +0.13f, +1.0f, 1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, +0.40f, +0.26f, +0.13f, +1.0f, 0.0f, 0.0f,
	};

	// Creates the Vertex Attribute Pointer
	const GLuint floatsPerVertex = 3; // Number of coordinates per vertex
	const GLuint floatsPerColor = 4; // (r,g,b,a)
	const GLuint floatsPerTexture = 2; // Texture

	shape.Vertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerColor + floatsPerTexture));

	glGenVertexArrays(1, &shape.vao);
	glGenBuffers(1, &shape.vbo);

	glBindVertexArray(shape.vao);

	glBindBuffer(GL_ARRAY_BUFFER, shape.vbo);  // Activates the buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);  // Sends vertex or coordinate data to the GPU

	// Strides between vertex coordinates is 6 (x, y, r, g, b, a). A tightly packed stride is 0.
	GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor + floatsPerTexture); // number of floats before each

	// pointer for the vertices
	glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);  // Position of coordinates in the buffer

	// pointer for the colors
	//glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
	glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
	glEnableVertexAttribArray(1); // Position of color data in buffer

	// pointer for the texture
	glVertexAttribPointer(2, floatsPerTexture, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerColor)));
	glEnableVertexAttribArray(2); // Position of texture data in buffer

}

void CreateOpenPyramid(GLShape& shape)
{
	//Vertex data
	GLfloat verts[] = {
		// Vertex Positions    // Colors            // Texture
		// Far side
		 0.0f,  0.0f,  0.0f, +0.40f, +0.26f, +0.13f, +1.0f, 0.5f, 1.0f,
		 0.5f, -0.5f,  0.5f, +0.40f, +0.26f, +0.13f, +1.0f, 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f, +0.40f, +0.26f, +0.13f, +1.0f, 0.0f, 0.0f,

		// Near side
		 0.0f,   0.0f,  0.0f, +0.40f, +0.26f, +0.13f, +1.0f, 0.5f, 1.0f,
		 0.5f,  -0.5f, -0.5f, +0.40f, +0.26f, +0.13f, +1.0f, 0.0f, 0.0f,
		 0.5f,   0.5f, -0.5f, +0.40f, +0.26f, +0.13f, +1.0f, 1.0f, 0.0f,

		// Left side
		 0.0f,  0.0f,  0.0f, +0.40f, +0.26f, +0.13f, +1.0f, 0.5f, 1.0f,
		 0.5f,  0.5f, -0.5f, +0.40f, +0.26f, +0.13f, +1.0f, 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f, +0.40f, +0.26f, +0.13f, +1.0f, 0.0f, 0.0f,

		// Right side
		 0.0f,  0.0f,  0.0f, +0.40f, +0.26f, +0.13f, +1.0f, 0.5f, 1.0f,
		 0.5f, -0.5f, -0.5f, +0.40f, +0.26f, +0.13f, +1.0f, 1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f, +0.40f, +0.26f, +0.13f, +1.0f, 0.0f, 0.0f,
	};

	// Creates the Vertex Attribute Pointer
	const GLuint floatsPerVertex = 3; // Number of coordinates per vertex
	const GLuint floatsPerColor = 4; // (r,g,b,a)
	const GLuint floatsPerTexture = 2; // Texture

	shape.Vertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerColor + floatsPerTexture));

	glGenVertexArrays(1, &shape.vao);
	glGenBuffers(1, &shape.vbo);

	glBindVertexArray(shape.vao);

	glBindBuffer(GL_ARRAY_BUFFER, shape.vbo);  // Activates the buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);  // Sends vertex or coordinate data to the GPU

	// Strides between vertex coordinates is 6 (x, y, r, g, b, a). A tightly packed stride is 0.
	GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor + floatsPerTexture); // number of floats before each

	// pointer for the vertices
	glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);  // Position of coordinates in the buffer

	// pointer for the colors
	//glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
	glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
	glEnableVertexAttribArray(1); // Position of color data in buffer

	// pointer for the texture
	glVertexAttribPointer(2, floatsPerTexture, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerColor)));
	glEnableVertexAttribArray(2); // Position of texture data in buffer

}

void setCoords(double r, double c, int rSeg, int cSeg, int i, int j,
	GLfloat* vertices, GLfloat* uv) {
	const double PI = 3.1415926535897932384626433832795;
	const double TAU = 2 * PI;

	double x = (c + r * cos(i * TAU / rSeg)) * cos(j * TAU / cSeg);
	double y = (c + r * cos(i * TAU / rSeg)) * sin(j * TAU / cSeg);
	double z = r * sin(i * TAU / rSeg);

	vertices[0] = 2 * x;
	vertices[1] = 2 * y;
	vertices[2] = 2 * z;

	uv[0] = i / (double)rSeg;
	uv[1] = j / (double)cSeg;
}

int createObject(double r, double c, int rSeg, int cSeg, GLfloat** vertices,
	GLfloat** uv) {
	int count = rSeg * cSeg * 6;
	*vertices = (GLfloat*)malloc(count * 3 * sizeof(GLfloat));
	*uv = (GLfloat*)malloc(count * 2 * sizeof(GLfloat));

	for (int x = 0; x < cSeg; x++) { // through stripes
		for (int y = 0; y < rSeg; y++) { // through squares on stripe
			GLfloat* vertexPtr = *vertices + ((x * rSeg) + y) * 18;
			GLfloat* uvPtr = *uv + ((x * rSeg) + y) * 12;
			setCoords(r, c, rSeg, cSeg, x, y, vertexPtr + 0, uvPtr + 0);
			setCoords(r, c, rSeg, cSeg, x + 1, y, vertexPtr + 3, uvPtr + 2);
			setCoords(r, c, rSeg, cSeg, x, y + 1, vertexPtr + 6, uvPtr + 4);

			setCoords(r, c, rSeg, cSeg, x, y + 1, vertexPtr + 9, uvPtr + 6);
			setCoords(r, c, rSeg, cSeg, x + 1, y, vertexPtr + 12, uvPtr + 8);
			setCoords(r, c, rSeg, cSeg, x + 1, y + 1, vertexPtr + 15,
				uvPtr + 10);
		}
	}

	return count;
}

void CreateTorus(GLTorus& torus)
{
	GLfloat* g_vertex_buffer_data;
	GLfloat* g_uv_buffer_data;
	int torusVertices = createObject(.5, 3.0, 180, 180, &g_vertex_buffer_data,
		&g_uv_buffer_data);

	torus.Vertices = torusVertices;

	glGenVertexArrays(1, &torus.vao);
	glGenBuffers(1, &torus.vbo);

	glBindVertexArray(torus.vao);

	glBindBuffer(GL_ARRAY_BUFFER, torus.vbo);  // Activates the buffer
	glBufferData(GL_ARRAY_BUFFER, torusVertices * 3 * sizeof(GLfloat), g_vertex_buffer_data, GL_STATIC_DRAW);

	glGenBuffers(1, &torus.uvbo);
	glBindBuffer(GL_ARRAY_BUFFER, torus.uvbo);
	glBufferData(GL_ARRAY_BUFFER, torusVertices * 2 * sizeof(GLfloat),
		g_uv_buffer_data, GL_STATIC_DRAW);


		// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, torus.vbo);
	glVertexAttribPointer(0, // attribute. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, torus.uvbo);
	glVertexAttribPointer(1, // attribute. No particular reason for 1, but must match the layout in the shader.
		2,                                // size : U+V => 2
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

}