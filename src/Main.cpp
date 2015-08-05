///////////////////////////////////////////
#include <vector>
#include "Core.h"
///////////////////////////////////////////
#include "RLE4.h"
///////////////////////////////////////////
#include "bitmap_fonts.h"
///////////////////////////////////////////
struct CUBE256
{

	vec3f pos,rot;

	struct MipMap
	{
		int handle;
		std::vector<uint> voxels; // x y<<8 z<<16 color<<24
	};
	std::vector<MipMap> mipmaps;

	//std::vector<uint> voxels; // x y<<8 z<<16 color<<24

	void set_voxel(int mipmap,uint x,uint y,uint z,uchar4 color)
	{
		if(x>255) return;if(x<0) return;
		if(y>255) return;if(x<0) return;
		if(z>255) return;if(x<0) return;
		if(mipmap>=mipmaps.size()) mipmaps.resize(mipmap+1);
		mipmaps[mipmap].voxels.push_back( uchar4(x,y,z,color.x).to_uint() );
	}
	void gen_vbo()
	{
		if(mipmaps.size()==0) return;

		loopi(0,mipmaps.size())
		if(mipmaps[i].voxels.size()>0)
		{
			//voxels.resize(voxels.size());
			glGenBuffers(1, (GLuint *)(&mipmaps[i].handle));
			glBindBuffer(GL_ARRAY_BUFFER, (GLuint)(mipmaps[i].handle));
			glBufferData(GL_ARRAY_BUFFER, mipmaps[i].voxels.size()*4,&mipmaps[i].voxels[0], GL_STATIC_DRAW_ARB );	
		}
	}
	void draw(int mipmap=0,vec3f add=vec3f(0,0,0))
	{
		if(mipmap>=mipmaps.size()) return;
		if(mipmaps[mipmap].voxels.size()==0) return;

		static Shader shader("Particle Shader");
		static bool init=true;

		if(init)
		{
			init=false;
			/*+++++++++++++++++++++++++++++++++++++*/
			shader.attach(GL_VERTEX_SHADER,"../shader/particle/vs.txt");
			shader.attach(GL_FRAGMENT_SHADER,"../shader/particle/frag.txt");
			shader.attach(GL_GEOMETRY_SHADER,"../shader/particle/geo.txt");
			shader.link();
			/*+++++++++++++++++++++++++++++++++++++*/
		}

		float Projection[16];
		float Modelview[16];
		glGetFloatv(GL_PROJECTION_MATRIX, Projection);		

		shader.begin();
		shader.setUniformMatrix4fv("projectionMatrix", 1, 0, Projection);	
						
		int vertexLocation0 = glGetAttribLocation(shader.program_handle, "vertex0");
		glEnableVertexAttribArray(vertexLocation0);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, mipmaps[mipmap].handle);			CHECK_GL_ERROR();
		glEnableClientState(GL_VERTEX_ARRAY);					CHECK_GL_ERROR();
		glVertexAttribPointer(vertexLocation0, 4, GL_UNSIGNED_BYTE, GL_TRUE, 16, 0);
						
		int vertexLocation1 = glGetAttribLocation(shader.program_handle, "vertex1");
		glEnableVertexAttribArray(vertexLocation1);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, mipmaps[mipmap].handle);			CHECK_GL_ERROR();
		glEnableClientState(GL_VERTEX_ARRAY);					CHECK_GL_ERROR();
		glVertexAttribPointer(vertexLocation1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 16, (void*)4);
						
		int vertexLocation2 = glGetAttribLocation(shader.program_handle, "vertex2");
		glEnableVertexAttribArray(vertexLocation2);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, mipmaps[mipmap].handle);			CHECK_GL_ERROR();
		glEnableClientState(GL_VERTEX_ARRAY);					CHECK_GL_ERROR();
		glVertexAttribPointer(vertexLocation2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 16, (void*)8);
						
		int vertexLocation3 = glGetAttribLocation(shader.program_handle, "vertex3");
		glEnableVertexAttribArray(vertexLocation3);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, mipmaps[mipmap].handle);			CHECK_GL_ERROR();
		glEnableClientState(GL_VERTEX_ARRAY);					CHECK_GL_ERROR();
		glVertexAttribPointer(vertexLocation3, 4, GL_UNSIGNED_BYTE, GL_TRUE, 16, (void*)12);
								
		vec3f pos_sum=add+pos;
		shader.setUniform4f("block_pos",pos_sum.x,pos_sum.y,pos_sum.z,1 );	
		shader.setUniform1f("scale",0.1f*float(1<<mipmap) );	
		glDrawArrays( GL_POINTS, 0, mipmaps[mipmap].voxels.size()/4);

		glDisableVertexAttribArray(vertexLocation0);
		glDisableVertexAttribArray(vertexLocation1);
		glDisableVertexAttribArray(vertexLocation2);
		glDisableVertexAttribArray(vertexLocation3);
		glDisableClientState(GL_VERTEX_ARRAY);									CHECK_GL_ERROR();
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);								CHECK_GL_ERROR();

		shader.end();
	}
};
int cubes_x = 4;
int cubes_y = 4;
int cubes_z = 4;
std::vector<CUBE256> cubes;

void init_cubes()
{
	cubes.resize(cubes_x*cubes_y*cubes_z);
	
	int c=0;
	loopk(0,cubes_z)
	loopj(0,cubes_y)
	loopi(0,cubes_x)
	{
		cubes[c++].pos=vec3f(i,j,k);
	}
}
void set_voxel(int mipmap,uint x,uint y,uint z,uchar4 color)
{
	if(x>cubes_x*256)return;
	if(y>cubes_y*256)return;
	if(z>cubes_z*256)return;
	x<<=mipmap;
	y<<=mipmap;
	z<<=mipmap;
	int id=(x>>8)+(y>>8)*cubes_x+(z>>8)*cubes_x*cubes_y;
	cubes[id].set_voxel(mipmap,x&255,y&255,z&255,color);
}
///////////////////////////////////////////
void DrawScene()
{
	static int framenumber=0;

	glViewport(0,0,screen.window_width,screen.window_height);
	glClearDepth(1.0f);
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode( GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(screen.fov, (GLfloat)screen.window_width / (GLfloat) screen.window_height, 0.01, 10000.0);
	
	// Position
	glRotatef(screen.rot.z, 0,0,1);
	glRotatef(screen.rot.y, 1,0,0);
	glRotatef(screen.rot.x, 0,1,0);
	glTranslatef(
		screen.pos.x,
		screen.pos.y,
		screen.pos.z
		);

	// Particle
	glScalef(16,16,16);

	glMatrixMode( GL_MODELVIEW);
	glLoadIdentity();
	
	loopi(0,cubes.size())
	loopk(0,4)
	loopj(0,4)
	{
		vec3f add=vec3f(cubes_x*k,0,cubes_y*j);
		vec3f pos=cubes[i].pos;

		//int mipmap = log(1+(screen.pos-(pos+vec3f(0.5,0.5,0.5)-add*16)).length()/16) / log( 2.0f );
		int mipmap = sqrt(sqrt((screen.pos+((pos+vec3f(0.5,0.5,0.5)+add)*16)).length()))-1;
		if(mipmap<0)mipmap=0;
		
		cubes[i].draw(mipmap,add);
	}
	
	//OSD
	
	beginRenderText( screen.window_width, screen.window_height);
    {
		char text[100];	int HCOL= 0,VCOL = 5; glColor3f( 1.0f, 1.0f, 1.0f );
		
		sprintf(text, "FrameTime / FPS / MTri/s:  %2.2fms %2.2f" , screen.screentime,1.0/screen.screentime );											renderText( VCOL, HCOL+=15, BITMAP_FONT_TYPE_HELVETICA_12, text );
		sprintf(text, "Resolution : %d %d", screen.window_width, screen.window_height);			renderText( VCOL, HCOL+=15, BITMAP_FONT_TYPE_HELVETICA_12, text );
		
	}
    endRenderText();
	

	printf("pos %2.2f %2.2f %2.2f rot %2.2f %2.2f %2.2f \r",		
		screen.pos.x,
		screen.pos.y,
		screen.pos.z,
		screen.rot.x,
		screen.rot.y,
		screen.rot.z
		);

	CoreKeyMouse();

	glutSwapBuffers();
}
///////////////////////////////////////////
int main(int argc, char **argv) 
{ 
	printf("Loading ...\n");

	RLE4 rle;

	init_cubes();
//	rle.load_m5("../voxelstein.rle4");
	rle.load_m5("../sphereN.rle4");

	printf("\n\n#Cubes %d\n\n",cubes.size());

	int sum=0;
	loopi(0,cubes.size()) 
	if(cubes[i].mipmaps.size()>0)
		sum+=cubes[i].mipmaps[0].voxels.size();
	printf("\n\nvoxels total:%d Million\n\n",sum/(1024*1024));

	screen.window_width = SCREEN_SIZE_X;
	screen.window_height = SCREEN_SIZE_Y;
	CoreInit(DrawScene,argc, argv);
	wglSwapIntervalEXT(0);

	loopi(0,cubes.size()) cubes[i].gen_vbo();

	printf("main loop\n");
	glutMainLoop();
}
///////////////////////////////////////////