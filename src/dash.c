#include "dash.h"
#include "obj_parser.h"
#include "gl_utils.h"
#include "camera.h"
#include "stb_image.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

#define DASH_MESH "meshes/dash.obj"
#define MIRROR_MESH "meshes/mirror.obj"
#define STEERING_MESH "meshes/steering.obj"
#define DASH_VS "shaders/dash.vert"
#define DASH_FS "shaders/dash.frag"
#define MIRROR_VS "shaders/mirror.vert"
#define MIRROR_FS "shaders/mirror.frag"
#define MIRROR_OUTER_VS "shaders/mirror_outer.vert"
#define MIRROR_OUTER_FS "shaders/mirror_outer.frag"
#define STEERING_VS "shaders/steering.vert"
#define STEERING_FS "shaders/steering.frag"
#define STEERING_DIFF "textures/steering.png"

GLuint dash_vao, mirror_vao, steering_vao;
int dash_point_count, mirror_point_count, steering_point_count;

GLuint dash_sp, mirror_sp, mirror_outer_sp, steering_sp;
GLint dash_M_loc, dash_V_loc, dash_P_loc;
GLint dash_w_loc, dash_h_loc;
GLint mirror_M_loc, mirror_V_loc,  mirror_P_loc;
GLint mirror_outer_M_loc, mirror_outer_V_loc,  mirror_outer_P_loc;
GLint steering_M_loc, steering_V_loc, steering_P_loc;
GLint steering_w_loc, steering_h_loc;
GLuint steering_diff_map;
float steering_deg;

mat4 dash_M, mirror_M, mirror_outer_M, steering_M, dash_V, P_boring;
vec3 dash_pos;

bool init_dash () {
	float* points = NULL;
	float* tex_coords = NULL;
	float* normals = NULL;
	GLuint vp_vbo, vt_vbo, vn_vbo;

	printf ("Init dashboard...\n");
	
	dash_M = identity_mat4 ();
	mirror_M = identity_mat4 ();
	mirror_outer_M = identity_mat4 ();
	steering_M = identity_mat4 ();

	// custom look-at for dashboard
	dash_V = look_at (vec3 (0.0f, 0.0f, 0.0f), vec3 (0.0f, 0.0f, -1.0f),
		vec3 (0.0f, 1.0f, 0.0f));
	
	P_boring = perspective (
			67.0f, (float)gl_width / (float)gl_height, 0.1f, 200.0f);
	
	if (!load_obj_file (
		DASH_MESH,
		points,
		tex_coords,
		normals,
		dash_point_count
	)) {
		fprintf (stderr, "ERROR loading dashboard mesh\n");
		return false;
	}
	
	glGenBuffers (1, &vp_vbo);
	glGenBuffers (1, &vt_vbo);
	glGenBuffers (1, &vn_vbo);
	glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
	glBufferData (GL_ARRAY_BUFFER, dash_point_count * sizeof (float) * 3,
		points, GL_STATIC_DRAW);
	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	glBufferData (GL_ARRAY_BUFFER, dash_point_count * sizeof (float) * 2,
		tex_coords, GL_STATIC_DRAW);
	glBindBuffer (GL_ARRAY_BUFFER, vn_vbo);
	glBufferData (GL_ARRAY_BUFFER, dash_point_count * sizeof (float) * 3,
		normals, GL_STATIC_DRAW);
	
	free (points);
	free (tex_coords);
	free (normals);
	
	glGenVertexArrays (1, &dash_vao);
	glBindVertexArray (dash_vao);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (1);
	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (2);
	glBindBuffer (GL_ARRAY_BUFFER, vn_vbo);
	glVertexAttribPointer (2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	if (!load_obj_file (
		MIRROR_MESH,
		points,
		tex_coords,
		normals,
		mirror_point_count
	)) {
		fprintf (stderr, "ERROR loading mirror mesh\n");
		return false;
	}
	
	glGenBuffers (1, &vp_vbo);
	glGenBuffers (1, &vt_vbo);
	glGenBuffers (1, &vn_vbo);
	glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
	glBufferData (GL_ARRAY_BUFFER, mirror_point_count * sizeof (float) * 3,
		points, GL_STATIC_DRAW);
	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	glBufferData (GL_ARRAY_BUFFER, mirror_point_count * sizeof (float) * 2,
		tex_coords, GL_STATIC_DRAW);
	glBindBuffer (GL_ARRAY_BUFFER, vn_vbo);
	glBufferData (GL_ARRAY_BUFFER, mirror_point_count * sizeof (float) * 3,
		normals, GL_STATIC_DRAW);
		
	glGenVertexArrays (1, &mirror_vao);
	glBindVertexArray (mirror_vao);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (1);
	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (2);
	glBindBuffer (GL_ARRAY_BUFFER, vn_vbo);
	glVertexAttribPointer (2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	free (points);
	free (tex_coords);
	free (normals);
	
	//
	// steering wheel
	//
	if (!load_obj_file (
		STEERING_MESH,
		points,
		tex_coords,
		normals,
		steering_point_count
	)) {
		fprintf (stderr, "ERROR loading steering mesh\n");
		return false;
	}
	
	glGenBuffers (1, &vp_vbo);
	glGenBuffers (1, &vt_vbo);
	glGenBuffers (1, &vn_vbo);
	glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
	glBufferData (GL_ARRAY_BUFFER, steering_point_count * sizeof (float) * 3,
		points, GL_STATIC_DRAW);
	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	glBufferData (GL_ARRAY_BUFFER, steering_point_count * sizeof (float) * 2,
		tex_coords, GL_STATIC_DRAW);
	glBindBuffer (GL_ARRAY_BUFFER, vn_vbo);
	glBufferData (GL_ARRAY_BUFFER, steering_point_count * sizeof (float) * 3,
		normals, GL_STATIC_DRAW);
	
	glGenVertexArrays (1, &steering_vao);
	glBindVertexArray (steering_vao);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (1);
	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (2);
	glBindBuffer (GL_ARRAY_BUFFER, vn_vbo);
	glVertexAttribPointer (2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	// shader
	dash_sp = link_programme_from_files (DASH_VS, DASH_FS);
	dash_P_loc = glGetUniformLocation (dash_sp, "P");
	dash_V_loc = glGetUniformLocation (dash_sp, "V");
	dash_M_loc = glGetUniformLocation (dash_sp, "M");
	dash_w_loc = glGetUniformLocation (dash_sp, "w");
	dash_h_loc = glGetUniformLocation (dash_sp, "h");
	glUseProgram (dash_sp);
	glUniformMatrix4fv (dash_V_loc, 1, GL_FALSE, dash_V.m);
	
	mirror_sp = link_programme_from_files (MIRROR_VS, MIRROR_FS);
	mirror_P_loc = glGetUniformLocation (mirror_sp, "P");
	mirror_V_loc = glGetUniformLocation (mirror_sp, "V");
	mirror_M_loc = glGetUniformLocation (mirror_sp, "M");
	glUseProgram (mirror_sp);
	glUniformMatrix4fv (mirror_V_loc, 1, GL_FALSE, dash_V.m);
	
	mirror_outer_sp = link_programme_from_files (
		MIRROR_OUTER_VS, MIRROR_OUTER_FS);
	mirror_outer_P_loc = glGetUniformLocation (mirror_outer_sp, "P");
	mirror_outer_V_loc = glGetUniformLocation (mirror_outer_sp, "V");
	mirror_outer_M_loc = glGetUniformLocation (mirror_outer_sp, "M");
	glUseProgram (mirror_outer_sp);
	glUniformMatrix4fv (mirror_outer_V_loc, 1, GL_FALSE, dash_V.m);
	
	steering_sp = link_programme_from_files (STEERING_VS, STEERING_FS);
	steering_P_loc = glGetUniformLocation (steering_sp, "P");
	steering_V_loc = glGetUniformLocation (steering_sp, "V");
	steering_M_loc = glGetUniformLocation (steering_sp, "M");
	steering_w_loc = glGetUniformLocation (steering_sp, "w");
	steering_h_loc = glGetUniformLocation (steering_sp, "h");
	glUseProgram (steering_sp);
	glUniformMatrix4fv (steering_V_loc, 1, GL_FALSE, dash_V.m);
	
	// textures
	steering_diff_map = create_texture_from_file (STEERING_DIFF);
	
	dash_M = translate (identity_mat4 (), vec3 (0.0f, 0.0f, -1.125f));
	
	mirror_M = scale (identity_mat4 (), vec3 (0.4f, 0.4f, 0.4f));
	mirror_M = translate (mirror_M, vec3 (0.65f, 0.6f, -1.125f));
	
	mirror_outer_M = scale (identity_mat4 (), vec3 (0.45f, 0.45f, 0.45f));
	mirror_outer_M = translate (mirror_outer_M, vec3 (0.65f, 0.6f, -1.125f));
	
	return true;
}

void set_steering (float deg) {
	mat4 R;
	
	steering_deg = deg;
	R = rotate_z_deg (identity_mat4 (), steering_deg);
	steering_M = translate (R, vec3 (0.0f, -0.7f, -1.125f));
}

void move_dash (vec3 p) {
	dash_pos = p;
}

void draw_dash () {
	glUseProgram (dash_sp);
	//if (cam_V_dirty) {
	//	glUniformMatrix4fv (dash_V_loc, 1, GL_FALSE, V.m);
	//}
	if (cam_P_dirty) {
		P_boring = perspective (
			67.0f, (float)gl_width / (float)gl_height, 0.1f, 200.0f);
		glUniformMatrix4fv (dash_P_loc, 1, GL_FALSE, P_boring.m);
		glUniform1f (dash_h_loc, (float)gl_height);
		glUniform1f (dash_w_loc, (float)gl_width);
	}
	
	glUniformMatrix4fv (dash_M_loc, 1, GL_FALSE, dash_M.m);
	glBindVertexArray (dash_vao);
	glDrawArrays (GL_TRIANGLES, 0, dash_point_count);
	
	glUseProgram (steering_sp);
	//if (cam_V_dirty) {
	//	glUniformMatrix4fv (steering_V_loc, 1, GL_FALSE, V.m);
	//}
	if (cam_P_dirty) {
		P_boring = perspective (
			67.0f, (float)gl_width / (float)gl_height, 0.1f, 200.0f);
		glUniformMatrix4fv (steering_P_loc, 1, GL_FALSE, P_boring.m);
		glUniform1f (steering_h_loc, (float)gl_height);
		glUniform1f (steering_w_loc, (float)gl_width);
	}
	
	//
	// steering wheel
	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, steering_diff_map);
	glUniformMatrix4fv (steering_M_loc, 1, GL_FALSE, steering_M.m);
	glBindVertexArray (steering_vao);
	glDrawArrays (GL_TRIANGLES, 0, steering_point_count);
	
	//
	// mirror
	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, cam_mirror_tex);
	// outside black bit of mirror
	glUseProgram (mirror_outer_sp);
	//if (cam_V_dirty) {
	//	glUniformMatrix4fv (mirror_outer_V_loc, 1, GL_FALSE, V.m);
	//}
	if (cam_P_dirty) {
		glUniformMatrix4fv (mirror_outer_P_loc, 1, GL_FALSE, P_boring.m);
	}
	glUniformMatrix4fv (mirror_outer_M_loc, 1, GL_FALSE, mirror_outer_M.m);
	glBindVertexArray (mirror_vao);
	glDrawArrays (GL_TRIANGLES, 0, mirror_point_count);
	
	glUseProgram (mirror_sp);
	//if (cam_V_dirty) {
	//	glUniformMatrix4fv (mirror_V_loc, 1, GL_FALSE, V.m);
	//}
	if (cam_P_dirty) {
		glUniformMatrix4fv (mirror_P_loc, 1, GL_FALSE, P_boring.m);
	}
	
	glUniformMatrix4fv (mirror_M_loc, 1, GL_FALSE, mirror_M.m);
	glBindVertexArray (mirror_vao);
	glDrawArrays (GL_TRIANGLES, 0, mirror_point_count);
}
