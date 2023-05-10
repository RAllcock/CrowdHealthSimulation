#include "particles.h"

int Particles::setup(int pn, int pnmax, glm::vec2 pb, glm::ivec2 pbs, glm::vec2 rg, float maxSpd, float maxAcc, float sdist){
	
	// input
	n = pn;
	nmax = pnmax;
	maxSpeed = maxSpd;
	maxAcceleration = maxAcc;
	defaultGoalPos = rg;
	
	// memory allocation
	datan = 6 * n;
	datanmax = 6 * nmax;
	if(	(position = (glm::vec2*)malloc(sizeof(glm::vec2) * nmax)) == NULL ||
		(velocity = (glm::vec2*)malloc(sizeof(glm::vec2) * nmax)) == NULL ||
		(acceleration = (glm::vec2*)malloc(sizeof(glm::vec2) * nmax)) == NULL ||
		(acc1 = (glm::vec2*)malloc(sizeof(glm::vec2) * nmax)) == NULL ||
		(acc2 = (glm::vec2*)malloc(sizeof(glm::vec2) * nmax)) == NULL ||
		(acc3 = (glm::vec2*)malloc(sizeof(glm::vec2) * nmax)) == NULL ||
		(goalPos = (glm::vec2*)malloc(sizeof(glm::vec2) * nmax)) == NULL ||
		(disc = (float*)malloc(sizeof(float) * nmax)) == NULL ||
		(mass = (float*)malloc(sizeof(float) * nmax)) == NULL ||
		(density = (float*)malloc(sizeof(float) * nmax)) == NULL ||
		(data = (GLfloat*)malloc(sizeof(GLfloat) * datanmax)) == NULL){
		
		printf("Particle allocation failed\n");
		return -1;
	}
	
	// shader
	setShader();
	
	// data
	set(pb, pbs, rg, sdist);
	updateData();
	
	return 0;
}

void Particles::setScaling(glm::vec2 *sm, glm::vec2 *sp){
	mapScale = sm;
	particleScale = sp;
	
	// shader transformations
	scale = glm::mat4(
		1.f / (mapScale->x * particleScale->x), 0, 0, 0,
		0, 1.f / (mapScale->y * particleScale->y), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);
	projection = glm::ortho(-1, 1, -1, 1, -1, 100);
}

void Particles::set(glm::vec2 p, glm::ivec2 s, glm::vec2 rg, float spawnDist){
	
	// set particles
	for(int i = 0; i < n; i++){
		
		// place inside spawning region
		position[i] = glm::vec2(
			spawnDist * .5f + p.x + fmod(spawnDist * i, s.x),
			spawnDist * .5f + p.y + spawnDist * (int)(spawnDist * i/s.y));
		
		// zeroed values
		velocity[i] = glm::vec2(0);
		acceleration[i] = glm::vec2(0);
		
		// randomize size
		disc[i] = .215f + (float)(rand()%51) / 1000.f;
		mass[i] = pow(disc[i] / .24f, 2);
		
		// assign goal
		goalPos[i] = rg;
		
		// temporary
		density[i] = 0;
	}
	
	// clear data
	for(int i = 0; i < datan; i++) data[i] = 0;
}
void Particles::updateData(){
	int index = 0;
	for(int i = 0; i < n; i++){
		
		// position
		data[index++] = (position[i].x * 2.f / mapScale->x - 1.f) / particleScale->x;
		data[index++] = (position[i].y * 2.f / mapScale->y - 1.f) / particleScale->y;
		
		// size
		data[index++] = disc[i];
		
		// colour
		float d = (density[i] > 8 ? 1.25f*PI : 5.f*PI*density[i]/32.f);
		float r = -cos(d);
		float g = sin(d);
		float b = (density[i] < 4 ? cos(d) : -sin(d));
		data[index++] = (r < 0 ? 0 : r);
		data[index++] = (g < 0 ? 0 : g);
		data[index++] = (b < 0 ? 0 : b);
	}
	
	// update buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * datan, data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Particles::getShaderSources(const char **vShader, const char **gShader, const char **fShader){
	
	// vertex shader
	const char *vShaderSrcConst = 
		"#version 330 core\n"
		"layout (location = 0) in vec2 pos;"
		"layout (location = 1) in float r;"
		"layout (location = 2) in vec3 col;"
		"uniform mat4 projection;"
		"out vec3 vert_colour;"
		"out float vert_radius;"
		"void main(){"
			"vert_colour = col;"
			"vert_radius = r;"
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
		"in float vert_radius[];"
		"out vec3 geom_colour;"
		"void main(){"
			"vec2 curpoint = vec2(0, 2.0*vert_radius[0]);"
			"float rval = 3.1415 * (2.0 / 24.0);"
			"mat2 rotate = mat2("
				"cos(rval), sin(rval),"
				"-sin(rval), cos(rval)"
			");"
			"for(int i = 0; i < 23; i++){"
				
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
				"projection * scale * vec4(0, 2.0*vert_radius[0], 0, 0);"
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

void Particles::setShader(){
	
	// vertex buffer object
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * datanmax, NULL, GL_DYNAMIC_DRAW);
	
	// vertex array object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	// position
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, NULL);
	glEnableVertexAttribArray(0);
	
	// size
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (GLvoid*)(sizeof(GLfloat) * 2));
	glEnableVertexAttribArray(1);
	
	// colour
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (GLvoid*)(sizeof(GLfloat) * 3));
	glEnableVertexAttribArray(2);
	
	// shader code source
	const char *vShaderSrc, *gShaderSrc, *fShaderSrc;
	getShaderSources(&vShaderSrc, &gShaderSrc, &fShaderSrc);
	
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
	if((program = glCreateProgram()) == 0) printf("Shader program creation failed\n");
	glAttachShader(program, vs);
	glAttachShader(program, gs);
	glAttachShader(program, fs);
	
	// linking
	glLinkProgram(program);
	if(!(glGetProgramiv(program, GL_LINK_STATUS, &status), status)){
		glGetProgramInfoLog(program, sizeof(infoLog), NULL, infoLog);
		printf("Particle program linking failed: %s\n", infoLog);
	}
	
	// clear shaders
	glDeleteShader(vs);
	glDeleteShader(gs);
	glDeleteShader(fs);
}

void Particles::display(){
	
	// use pedestrian particle program
	glUseProgram(program);
	
	// update transformations
	glUniformMatrix4fv(glGetUniformLocation(program, "scale"), 1, GL_FALSE, glm::value_ptr(scale));
	glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	
	// draw buffer
	glBindVertexArray(vao);
	glDrawArrays(GL_POINTS, 0, n);
	
	// clear
	glBindVertexArray(0);
	glUseProgram(0);
}

void Particles::move(float timeStep){
	
	// move all particles
	for(int i = 0; i < n; i++){
		
		// summate and clamp acceleration
		acceleration[i] = acc1[i] + acc2[i] + acc3[i];
		acceleration[i] = (length(acceleration[i]) > maxAcceleration ? maxAcceleration * normalize(acceleration[i]) : acceleration[i]);
		
		// update and clamp velocity
		velocity[i] += acceleration[i] * timeStep;
		velocity[i] = (length(velocity[i]) > maxSpeed ? maxSpeed * normalize(velocity[i]) : velocity[i]);
		
		// update position
		position[i] += velocity[i] * timeStep;
	}
}

int Particles::add(glm::vec2 p){
	
	// halt creation if position is invalid or simulation is full
	if(n >= nmax || p.x == -1) return n;
	
	// new particle values
	position[n] = p;
	disc[n] = .215f + (float)(rand()%50) / 1000.f;
	mass[n] = pow(disc[n] / .24f, 2);
	
	// default goal destination
	goalPos[n] = defaultGoalPos;
	
	// zeroed values
	velocity[n] = glm::vec2(0);
	acceleration[n] = glm::vec2(0);
	
	// temporary
	density[n] = 0;
	
	// increment total particles
	n++;
	datan = 6 * n;
	
	return n;
}

int Particles::remove(int index){
	
	// halt deletion if simulation empty
	if(index < 0) return n;
	
	// decrement total particles
	n--;
	datan = 6 * n;
	
	// reassign outer particle index to removed particle index
	position[index] = position[n];
	velocity[index] = velocity[n];
	acceleration[index] = acceleration[n];
	acc1[index] = acc1[n];
	acc2[index] = acc2[n];
	acc3[index] = acc3[n];
	goalPos[index] = goalPos[n];
	disc[index] = disc[n];
	mass[index] = mass[n];
	density[index] = density[n];
	
	return n;
}

void Particles::cleanup(){
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteProgram(program);
	free(position);
	free(velocity);
	free(acceleration);
	free(acc1);
	free(acc2);
	free(acc3);
	free(goalPos);
	free(disc);
	free(mass);
	free(density);
	free(data);
}