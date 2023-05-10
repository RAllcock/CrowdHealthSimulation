#include "obstacles.h"

int Obstacles::setup(glm::vec2 *bpos, glm::vec2 *bsize, float d, float m, int ob, float eb){
	
	
	// general
	boxPos = bpos;
	boxSize = bsize;
	
	// input
	disc = d;
	mass = m;
	boxes = ob + eb;
	oboxes = ob;
	
	// count obstacle particles
	n = 0;
	for(int i = 0; i < oboxes; i++){
		float xdir = 2.f * disc * boxSize[i].x / fabs(boxSize[i].x);
		for(float x = xdir / 2.f; fabs(x + xdir/2.f) < fabs(boxSize[i].x); x += xdir){
			float ydir = 2.f * disc * boxSize[i].y / fabs(boxSize[i].y);
			for(float y = ydir / 2.f; fabs(y + ydir/2.f) < fabs(boxSize[i].y); y += ydir){
				n++;
			}
		}
	}
	
	// memory allocation
	odatan = 5 * n;
	bdatan = 12 * boxes;
	if(	(position = (glm::vec2*)malloc(sizeof(glm::vec2) * n)) == NULL || 
		(density = (float*)malloc(sizeof(float) * n)) == NULL || 
		(odata = (GLfloat*)malloc(sizeof(GLfloat) * odatan)) == NULL || 
		(bdata = (GLfloat*)malloc(sizeof(GLfloat) * bdatan)) == NULL){
			
		printf("Obstacle allocation failed\n");
		return -1;
	}
	
	// clear particle positions
	for(int i = 0; i < n; i++)
		position[i] = glm::vec2(0);
	
	// shaders
	setObstacleShader();
	setBoxShader();
	
	// particle data
	set();
	updateObstacleData();
	
	return 0;
}

void Obstacles::setScaling(glm::vec2 *ms, glm::vec2 *os, float scaleFactor){
	mapScale = ms;
	obstacleScale = os;
	
	// shader transformations
	oscale = glm::mat4(
		scaleFactor / (mapScale->x * obstacleScale->x), 0, 0, 0, 
		0, scaleFactor / (mapScale->y * obstacleScale->y), 0, 0, 
		0, 0, 1, 0, 
		0, 0, 0, 1);
	projection = glm::ortho(-1, 1, -1, 1, -1, 100);
}

void Obstacles::set(){
	
	// create obstacle particles inside boxes
	int index = 0;
	for(int i = 0; i < boxes; i++){
		float xdir = 2.f * disc * boxSize[i].x / fabs(boxSize[i].x);
		float ydir = 2.f * disc * boxSize[i].y / fabs(boxSize[i].y);
		for(float x = xdir / 2.f; fabs(x + xdir/2.f) < fabs(boxSize[i].x) && index < n; x += xdir){
			for(float y = ydir / 2.f; fabs(y + ydir/2.f) < fabs(boxSize[i].y) && index < n; y += ydir){
				position[index++] = glm::vec2(boxPos[i].x+x, boxPos[i].y+y);
			}
		}
	}
}

void Obstacles::setBoxData(){
	
	// set
	int index = 0;
	for(int i = 0; i < boxes; i++){
		float x = (boxSize[i].x < 0 ? boxPos[i].x + boxSize[i].x : boxPos[i].x);
		float y = (boxSize[i].y < 0 ? boxPos[i].y + boxSize[i].y : boxPos[i].y);
		float w = fabs(boxSize[i].x);
		float h = fabs(boxSize[i].y);
		
		// lower-left triangle
		bdata[index++] = x;
		bdata[index++] = y;
		bdata[index++] = x + w;
		bdata[index++] = y;
		bdata[index++] = x;
		bdata[index++] = y + h;
		
		// upper-right triangle
		bdata[index++] = x + w;
		bdata[index++] = y;
		bdata[index++] = x + w;
		bdata[index++] = y + h;
		bdata[index++] = x;
		bdata[index++] = y + h;
		
	}
	
	// transform
	index = 0;
	for(int i = 0; i < 6 * boxes; i++){
		bdata[index] = (bdata[index] * 2.f / mapScale->x - 1.f) / obstacleScale->x;
		index++;
		bdata[index] = (bdata[index] * 2.f / mapScale->y - 1.f) / obstacleScale->y;
		index++;
	}
}

void Obstacles::updateObstacleData(){
	
	// update data
	int index = 0;
	for(int i = 0; i < n; i++){
		
		// position
		odata[index++] = (position[i].x * 2.f / mapScale->x - 1.f) / obstacleScale->x;
		odata[index++] = (position[i].y * 2.f / mapScale->y - 1.f) / obstacleScale->y;
		
		// colour
		float d = (density[i] > 8 ? 1.25f*PI : 5.f*PI*density[i]/32.f);
		float r = -cos(d);
		float g = sin(d);
		float b = (density[i] < 4 ? cos(d) : -sin(d));
		odata[index++] = (r < 0 ? 0 : r);
		odata[index++] = (g < 0 ? 0 : g);
		odata[index++] = (b < 0 ? 0 : b);
	}
	
	// update buffer
	glBindBuffer(GL_ARRAY_BUFFER, ovbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * odatan, odata);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Obstacles::getObstacleShaderSources(const char **vShader, const char **gShader, const char **fShader){
	
	// vertex shader
	const char *vShaderSrcConst = 
		"#version 330 core\n"
		"layout (location = 0) in vec2 pos;"
		"layout (location = 1) in vec3 col;"
		"uniform mat4 projection;"
		"out vec3 vert_colour;"
		"void main(){"
			"vert_colour = col;"
			"gl_Position = projection * vec4(pos, 0, 1);"
		"}";
	
	// geometry shader
	const char *gShaderSrcConst = 
		"#version 330 core\n"
		"layout (points) in;"
		"layout (triangle_strip, max_vertices = 72) out;"
		"uniform mat4 scale;"
		"uniform mat4 projection;"
		"in vec3 vert_colour[];"
		"out vec3 geom_colour;"
		"void main(){"
			"vec2 curpoint = vec2(0, 1);"
			"float rval = 3.1415 * (2.0 / 24.0);"
			"mat2 rotate = mat2("
				"cos(rval), sin(rval),"
				"-sin(rval), cos(rval)"
			");"
			"for(int i = 0; i < 24; i++){"
				
				"gl_Position = gl_in[0].gl_Position;"
				"geom_colour = vert_colour[0];"
				"EmitVertex();"
				
				"gl_Position = gl_in[0].gl_Position + "
					"projection * scale * vec4(curpoint, 0, 0);"
				"geom_colour = vert_colour[0];"
				"EmitVertex();"
				
				"curpoint = rotate * curpoint;"
				"gl_Position = gl_in[0].gl_Position + "
					"projection * scale * vec4(curpoint, 0, 0);"
				"geom_colour = vert_colour[0];"
				"EmitVertex();"
				"EndPrimitive();"
				
			"}"
			"gl_Position = gl_in[0].gl_Position;"
			"geom_colour = vert_colour[0];"
			"EmitVertex();"
			
			"gl_Position = gl_in[0].gl_Position + "
				"projection * scale * vec4(curpoint, 0, 0);"
			"geom_colour = vert_colour[0];"
			"EmitVertex();"
			
			"curpoint = rotate * curpoint;"
			"gl_Position = gl_in[0].gl_Position + "
				"projection * scale * vec4(0, 1, 0, 0);"
			"geom_colour = vert_colour[0];"
			"EmitVertex();"
			"EndPrimitive();"
		"}";
	
	// fragment shader
	const char *fShaderSrcConst = 
		"#version 330 core\n"
		"in vec3 geom_colour;"
		"out vec4 frag_colour;"
		"void main(){"
			"frag_colour = vec4(geom_colour, 1);"
		"}";
	
	// output constants
	*vShader = vShaderSrcConst;
	*gShader = gShaderSrcConst;
	*fShader = fShaderSrcConst;
}

void Obstacles::setObstacleShader(){
	
	// vertex buffer object
	glGenBuffers(1, &ovbo);
	glBindBuffer(GL_ARRAY_BUFFER, ovbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * odatan, NULL, GL_DYNAMIC_DRAW);
	
	// vertex array object
	glGenVertexArrays(1, &ovao);
	glBindVertexArray(ovao);
	
	// position
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, NULL);
	glEnableVertexAttribArray(0);
	
	// colour
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, (GLvoid*)(sizeof(GLfloat) * 2));
	glEnableVertexAttribArray(1);
	
	// shader code source
	const char *vShaderSrc, *gShaderSrc, *fShaderSrc;
	getObstacleShaderSources(&vShaderSrc, &gShaderSrc, &fShaderSrc);
	
	// shader compilation
	GLint status;
	GLchar infoLog[1024];
	GLuint vs, gs, fs;
	
	// vertex shader
	vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vShaderSrc, NULL);
	glCompileShader(vs);
	if(!(glGetShaderiv(vs, GL_COMPILE_STATUS, &status), status)){
		glGetShaderInfoLog(vs, sizeof(infoLog), NULL, infoLog);
		printf("Vertex shader compilation failed: %s\n", infoLog);
	}
	
	// geometry shader
	gs = glCreateShader(GL_GEOMETRY_SHADER);
	glShaderSource(gs, 1, &gShaderSrc, NULL);
	glCompileShader(gs);
	if(!(glGetShaderiv(gs, GL_COMPILE_STATUS, &status), status)){
		glGetShaderInfoLog(gs, sizeof(infoLog), NULL, infoLog);
		printf("Geometry shader compilation failed: %s\n", infoLog);
	}
	
	// fragment shader
	fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fShaderSrc, NULL);
	glCompileShader(fs);
	if(!(glGetShaderiv(fs, GL_COMPILE_STATUS, &status), status)){
		glGetShaderInfoLog(fs, sizeof(infoLog), NULL, infoLog);
		printf("Fragment shader compilation failed: %s\n", infoLog);
	}
	
	// program creation
	if((oprogram = glCreateProgram()) == 0) printf("Shader program creation failed\n");
	glAttachShader(oprogram, vs);
	glAttachShader(oprogram, gs);
	glAttachShader(oprogram, fs);
	
	// linking
	glLinkProgram(oprogram);
	if(!(glGetProgramiv(oprogram, GL_LINK_STATUS, &status), status)){
		glGetProgramInfoLog(oprogram, sizeof(infoLog), NULL, infoLog);
		printf("Obstacle program linking failed: %s\n", infoLog);
	}
	
	// clear shaders
	glDeleteShader(vs);
	glDeleteShader(gs);
	glDeleteShader(fs);
}

void Obstacles::getBoxShaderSources(const char **vShader, const char **fShader){
	
	// vertex shader
	const char *vShaderSrcConst = 
		"#version 330 core\n"
		"layout (location = 0) in vec2 pos;"
		"uniform mat4 projection;"
		"void main(){"
			"gl_Position = projection * vec4(pos, 0, 1);"
		"}";
	
	// fragment shader
	const char *fShaderSrcConst = 
		"#version 330 core\n"
		"out vec4 frag_colour;"
		"void main(){"
			"frag_colour = vec4(0.6, 0.6, 0.6, 1);"
		"}";
	
	// output constants
	*vShader = vShaderSrcConst;
	*fShader = fShaderSrcConst;
}
void Obstacles::setBoxShader(){
	
	// data
	setBoxData();
	
	// vertex buffer object
	glGenBuffers(1, &bvbo);
	glBindBuffer(GL_ARRAY_BUFFER, bvbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * bdatan, bdata, GL_STATIC_DRAW);
	
	// vertex array object
	glGenVertexArrays(1, &bvao);
	glBindVertexArray(bvao);
	
	// position
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, NULL);
	glEnableVertexAttribArray(0);
	
	// shader code source
	const char *vShaderSrc,  *fShaderSrc;
	getBoxShaderSources(&vShaderSrc,  &fShaderSrc);
	
	// shader compilation
	GLint status;
	GLchar infoLog[1024];
	GLuint vs, fs;
	
	// vertex shader
	vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vShaderSrc, NULL);
	glCompileShader(vs);
	if(!(glGetShaderiv(vs, GL_COMPILE_STATUS, &status), status)){
		glGetShaderInfoLog(vs, sizeof(infoLog), NULL, infoLog);
		printf("Vertex shader compilation failed: %s\n", infoLog);
	}
	
	// fragment shader
	fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fShaderSrc, NULL);
	glCompileShader(fs);
	if(!(glGetShaderiv(fs, GL_COMPILE_STATUS, &status), status)){
		glGetShaderInfoLog(fs, sizeof(infoLog), NULL, infoLog);
		printf("Fragment shader compilation failed: %s\n", infoLog);
	}
	
	// program creation
	if((bprogram = glCreateProgram()) == 0) printf("Shader program creation failed\n");
	glAttachShader(bprogram, vs);
	glAttachShader(bprogram, fs);
	
	// linking
	glLinkProgram(bprogram);
	if(!(glGetProgramiv(bprogram, GL_LINK_STATUS, &status), status)){
		glGetProgramInfoLog(bprogram, sizeof(infoLog), NULL, infoLog);
		printf("Obstacle program linking failed: %s\n", infoLog);
	}
	
	// clear shaders
	glDeleteShader(vs);
	glDeleteShader(fs);
}

void Obstacles::display(){
	
	// use obstacle particle program
	glUseProgram(oprogram);
	
	// update transformations
	glUniformMatrix4fv(glGetUniformLocation(oprogram, "scale"), 1, GL_FALSE, glm::value_ptr(oscale));
	glUniformMatrix4fv(glGetUniformLocation(oprogram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	
	// draw buffer
	glBindVertexArray(ovao);
	glDrawArrays(GL_POINTS, 0, n);
	
	// use obstacle box program
	glUseProgram(bprogram);
	
	// update transformations
	glUniformMatrix4fv(glGetUniformLocation(bprogram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	
	// draw buffer
	glBindVertexArray(bvao);
	glDrawArrays(GL_TRIANGLES, 0, 6 * boxes);
	
	// clear
	glBindVertexArray(0);
	glUseProgram(0);
}

void Obstacles::cleanup(){
	glDeleteVertexArrays(1, &ovao);
	glDeleteBuffers(1, &ovbo);
	glDeleteProgram(oprogram);
	glDeleteVertexArrays(1, &bvao);
	glDeleteBuffers(1, &bvbo);
	glDeleteProgram(bprogram);
	free(position);
	free(density);
	free(odata);
	free(bdata);
}