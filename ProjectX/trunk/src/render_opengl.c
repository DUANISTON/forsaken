#ifdef OPENGL1
#include "render_gl_shared.h"
#include "new3d.h"
#include "lights.h"

extern render_info_t render_info;

// capabilities of the opengl system

static struct
{
	float anisotropic; // anisotropic filtering for viewing textures at a flat angle
} caps;

static void detect_caps( void )
{
	//check whether anisotropic filtering extension is supported
	caps.anisotropic = 0.0f;
	if(strstr((char*)glGetString(GL_EXTENSIONS), "GL_EXT_texture_filter_anisotropic"))
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &caps.anisotropic );
	DebugPrintf("render: anisotropic filtering support = %s\n",caps.anisotropic?"true":"false");
}

// poly modes

void render_mode_wireframe(void)
{
	glPolygonMode(GL_FRONT, GL_LINE);
	glPolygonMode(GL_BACK, GL_LINE);
}

void render_mode_points(void)
{
	glPolygonMode(GL_FRONT, GL_POINT);
	glPolygonMode(GL_BACK, GL_POINT);
}

void render_mode_fill(void)
{
	glPolygonMode(GL_FRONT, GL_FILL);
	glPolygonMode(GL_BACK, GL_FILL);
}

// unused in opengl
_Bool FSBeginScene(){ return true; }
_Bool FSEndScene(){ return true; }

// prototypes
void reset_trans( void );

// TODO	- should get this from gl caps ?
_Bool bSquareOnly = true;

//
// Texture Routines
//

u_int8_t gamma_table[256];

void build_gamma_table( double gamma )
{
	double k;
	int i;

	DebugPrintf("build_gamma_table( %f )\n",gamma);

#ifndef DEBUG_ON
	if (gamma <= 0)
		gamma = 1.0;
#endif

	k = 255.0/pow(255.0, 1.0/gamma);
	
	for (i = 0; i <= 255; i++)
	{
		gamma_table[i] = (u_int8_t)(k*(pow((double)i, 1.0/gamma)));
		if( i && !gamma_table[i] )
			gamma_table[i] = 1;
	}
}

void release_texture( LPTEXTURE texture ){
	if(!texture)
		return;
	glDeleteTextures( 1, texture );
	free(texture);
	texture = NULL;
}

_Bool create_texture(LPTEXTURE *t, const char *path, u_int16_t *width, u_int16_t *height, int numMips, _Bool * colorkey)
{
	GLuint * id = NULL;
	texture_image_t image;

	Change_Ext( path, image.path, ".PNG" );
	if( ! File_Exists( (char*) image.path ) )
	{
		DebugPrintf("Could not find texture file: %s\n",path);
		return true;
	}

	if(load_image( &image, numMips )!=0)
	{
		DebugPrintf("couldn't load image\n");
		return false;
	}

	// return values
	*width  = (u_int16_t) image.w;
	*height = (u_int16_t) image.h;
	(*colorkey) = (_Bool) image.colorkey;

	// employ colour key and gamma correction
	{
		int y, x;
		int size = 4;
		int pitch = size*image.w;
		for (y = 0; y < image.h; y++)
		{
			for (x = 0; x < image.w; x++)
			{
				// move to the correct offset in the data
				// y is the row and pitch is the size of a row
				// (x*size) is the length of each pixel data (column)
				DWORD index = (y*pitch)+(x*size);

				// image.data is packed in rgba
				image.data[index]   = (char) gamma_table[ (u_int8_t) image.data[index]];	   // red
				image.data[index+1] = (char) gamma_table[ (u_int8_t) image.data[index+1]];  // green
				image.data[index+2] = (char) gamma_table[ (u_int8_t) image.data[index+2]];  // blue
				image.data[index+3] = (char) gamma_table[ (u_int8_t) image.data[index+3]];  // alpha

				// colour key
				if( image.colorkey && (image.data[index] + image.data[index+1] + image.data[index+2]) == 0 )
					image.data[index+3] = 0; // alpha - pixel will not be rendered do to alpha value tests

			}
		}
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// create a new opengl texture
	if( ! *t )
	{
		id = malloc(sizeof(GLuint));
		*t = (void*)id;
		glGenTextures(1, id);
		glBindTexture(GL_TEXTURE_2D, *id);
	}
	// updates an existing texture
	else
	{
		id = *t;
		glBindTexture(GL_TEXTURE_2D, *id);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.w, image.h, GL_RGBA, GL_UNSIGNED_BYTE, image.data);
	}

	// when texture area is small, bilinear filter the closest mipmap
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
	// when texture area is large, bilinear filter the original
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	// the texture wraps over at the edges (repeat)
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	// anisotropic settings
	if(caps.anisotropic)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, caps.anisotropic);

	// generates full range of mipmaps and scales to nearest power of 2
	if(gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, image.w, image.h, GL_RGBA, GL_UNSIGNED_BYTE, image.data) != 0)
	{
	   CHECK_GL_ERRORS;
	   return false;
	}

	DebugPrintf( "Created texture: file=%s, width=%d, height=%d, colorkey=%s\n", 
		image.path, image.w, image.h, (image.colorkey ? "true" : "false") );

	destroy_image( &image );

	return true;
}

_Bool update_texture_from_file(LPTEXTURE dstTexture, const char *fileName, u_int16_t *width, u_int16_t *height, int numMips, _Bool * colorkey)
{
	create_texture(&dstTexture, fileName, width, height, numMips, colorkey);
	return true;
}

_Bool FSCreateTexture(LPTEXTURE *texture, const char *fileName, u_int16_t *width, u_int16_t *height, int numMips, _Bool * colourkey)
{	
	return create_texture(texture, fileName, width, height, numMips, colourkey);
}

static void print_info( void )
{
	GLboolean b;
	glGetBooleanv(GL_STEREO,&b);

	DebugPrintf( "gl vendor='%s', renderer='%s', version='%s', shader='%s', stereo='%s'\n",
		glGetString(GL_VENDOR),
		glGetString(GL_RENDERER),
		glGetString(GL_VERSION),
		glGetString(GL_SHADING_LANGUAGE_VERSION),
		(b)?"true":"false");

	DebugPrintf( "extensions='%s'\n", glGetString(GL_EXTENSIONS));
}

static void set_defaults( void )
{
	build_gamma_table(1.0f); // 1.0f means no gamma change
	glShadeModel(GL_SMOOTH); // TODO - is there gouraud ?
	glDisable(GL_LIGHTING); // we light our own verts
	reset_cull(); // default cull
	reset_trans(); // default blending
	glPolygonMode(GL_BACK, GL_NONE); // don't draw back faces
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	set_normal_states(); // default preset render mode
}

static void resize_viewport( int width, int height )
{
	render_viewport_t viewport;
	viewport.X = 0;
	viewport.Y = 0;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinZ = 0.0f;
	viewport.MaxZ = 1.0f;
	FSSetViewPort(&viewport);
}

// windows needs explicit retrieval of newer GL functions...
/*
#ifdef WIN32

static PFNGLBLENDCOLOREXTPROC glBlendColor = NULL;

void no_glBlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha){}

static void bind_glBlendColor(void)
{
	if( !glBlendColor ) glBlendColor = SDL_GL_GetProcAddress( "glBlendColor" );
	if( !glBlendColor ) glBlendColor = SDL_GL_GetProcAddress( "glBlendColorEXT" );
	if( !glBlendColor )
	{
		glBlendColor = (PFNGLBLENDCOLOREXTPROC) no_glBlendColor;
		DebugPrintf("bind_glBlendColor: failed to get proc address\n");
	}
}

static void bind_gl_funcs(void)
{
	bind_glBlendColor();
}

#endif // WIN32
*/

_Bool render_init( render_info_t * info )
{
/*
#ifdef WIN32
	bind_gl_funcs();
#endif // WIN32
*/
	print_info();
	detect_caps();
	set_defaults();
	resize_viewport(info->ThisMode.w, info->ThisMode.h);
	if(info->wireframe)
		render_mode_wireframe();
	info->ok_to_render = true;
	return true;
}

void render_cleanup( render_info_t * info )
{
	info->ok_to_render = false;
	// ???
}

extern _Bool sdl_init_video( void );
_Bool render_mode_select( render_info_t * info )
{
	render_cleanup( info );
	if(!sdl_init_video())
		return false;
	//if(!render_init( info ))
	//	return false;
	return true;
}

// TODO - in d3d9 render_flip would detect a lost device
//		lost as in alt+tab (etc) which caused video memory to dump
//		at this point we should set needs_reset = true

static _Bool needs_reset = false;

_Bool render_reset( render_info_t * info )
{
	if(!needs_reset)
		return false;
	if(!render_mode_select( info ))
		return false;
	needs_reset = false;
	return true;
}

void render_set_filter( _Bool red, _Bool green, _Bool blue )
{
	glColorMask(red?1:0, green?1:0, blue?1:0, 1);
}

_Bool render_flip( render_info_t * info )
{
	SDL_GL_SwapBuffers();
	CHECK_GL_ERRORS;
	return true;
}

void reset_trans( void )
{
	glDisable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_ZERO); // src, dest
}

void reset_zbuff( void )
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE); // depth write
}

void disable_zbuff_write( void )
{
	glDepthMask(GL_FALSE); // depth write
}

void disable_zbuff( void )
{
	glDisable(GL_DEPTH_TEST);
}

void cull_none( void )
{
	glDisable(GL_CULL_FACE);
}

void cull_cw( void )
{
	glCullFace(GL_FRONT); // cw is the front for us
}

void reset_cull( void )
{	
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
}

// accept fragment if alpha value is greater than x
// alpha values are from 0-255 (8bit units)
// glAlphaFunc expects the number as a fraction
// if your images have an alpha value less than x
// then they will be ignored during rendering !!!

void set_alpha_ignore( void )
{
	float x = 100.f;
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER,(x/255.0f));
}

void unset_alpha_ignore( void )
{
	glDisable(GL_ALPHA_TEST);
}

void set_normal_states( void )
{
	reset_zbuff();
	reset_trans();
}

void set_trans_state_9()
{
	glBlendFunc(GL_SRC_ALPHA,GL_ONE); // src, dest
}

void set_alpha_states( void )
{
	disable_zbuff_write();
	glEnable(GL_BLEND);
	set_trans_state_9();
}

extern float framelag;

// 16.7 ~ 1/85 * 71 / 0.05;
// 85 is my fps, 71 is the framelag multiplier, 0.05 is a suitable
// alpha for that fps.

// cooler white out effect made by lion but mostly implemented as software
// by most drivers causing the game to slow down allot
void set_whiteout_state( void )
{
	disable_zbuff_write();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE); // src, dest
/*
	// higher = more white; < 1.0 makes it darker
	float whiteness = 5.0f;

	float src_a = framelag / 16.7f;
	float dst_a = src_a / whiteness;

	glEnable(GL_BLEND);
	glBlendFunc(GL_CONSTANT_ALPHA,GL_ONE_MINUS_CONSTANT_COLOR); // src, dest
	glBlendColor(dst_a, dst_a, dst_a, src_a); // src, dest
*/
}

// TODO - is the stencil buffer ever cleared ?
// TODO - do we even use the stencil buffer ?
// TODO - FSClear is meant to clear current viewport
//        perhaps we can automate and remove need for rect arg ?

// clears color/zbuff same time to opaque black
_Bool FSClear(XYRECT * rect)
{
	int width, height, x, y;
	width = rect->x2 - rect->x1;
	height = rect->y2 - rect->y1;
	x = rect->x1;
	y = render_info.ThisMode.h - rect->y1 - height;
	// here we employ a stencil buffer so that we
	// only clear the desired part of the screen
	glEnable(GL_SCISSOR_TEST);
	glScissor(x, y, width, height);
	//
	glClearDepth(1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	//
	glDisable(GL_SCISSOR_TEST);
	return true;
}

_Bool FSClearBlack(void)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	return true;
}

_Bool FSClearDepth(XYRECT * rect)
{
	glClearDepth(1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);
	return true;
}

_Bool FSGetViewPort(render_viewport_t *view)
{
	GLint i[4];
	GLfloat f[2];
	// scalex/y are not modified here
	// xywh
	glGetIntegerv( GL_VIEWPORT, i );
	view->X	= i[0];
	view->Y	= render_info.ThisMode.h - (i[1] + i[3]);
	view->Width	= i[2];
	view->Height = i[3];
	// near,far
	glGetFloatv( GL_DEPTH_RANGE, f );
	view->MinZ = f[0];
	view->MaxZ = f[1];
	return true;
}

// TODO - we can probably use glScalef and glTranslatef
//        to invert the viewport dimentions

_Bool FSSetViewPort(render_viewport_t *view)
{
	// render_viewport_t x/y starts top/left
	// but glViewport starts from bottom/left
	int bottom = render_info.ThisMode.h - (view->Y + view->Height);
	glViewport(	view->X, bottom, (GLint) view->Width, (GLint) view->Height	);
	// sets the min/max depth values to render
	// default is max 1.0f and min 0.0f
	// this is here for compatibility with d3d9
	glDepthRange(view->MinZ,view->MaxZ);
	// i want to know if this is ever changed
	// as most likely we don't need this info in render_viewport_t
	if(view->MaxZ!=1.0f || view->MinZ!=0.0f)
	{
		DebugPrintf("-------------------------------\n");
		DebugPrintf("max/min z used: max=%d min=%d\n",
			view->MaxZ, view->MinZ);
		DebugPrintf("-------------------------------\n");
	}
	// ScaleX|Y are not even part of the d3d9 struct anymore
	// they were part of the old d3d6 viewport settings
	// from testing d3d9 passes the values along untouched
	// probably need some testing here to see how d3d6 worked
	// forsaken still uses these values so we need them
	return true;
}

GLfloat proj_matrix[4][4];
_Bool FSSetProjection( RENDERMATRIX *matrix )
{
	memmove(&proj_matrix,&matrix->m,sizeof(proj_matrix));//memcpy
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf((GLfloat*)&matrix->m);
	return true;
}

//
// d3d stored the world/view matrixes
// and then multiplied them together before rendering
// in the following order: world * view * projection
// opengl handles only world and projection
// so we must emulate the behavior of world*view
//
// although we multiply the arguments backwards view*world
// other wise instead of an object rotating you end up turning the world
// causing pickups etc.. to fly around the entire level wicked fast!
//

GLfloat view_matrix[4][4];
GLfloat world_matrix[4][4];
GLfloat mv_matrix[4][4];

static void reset_modelview( void )
{
	glMatrixMode(GL_MODELVIEW);
	MatrixMultiply( &world_matrix, &view_matrix, &mv_matrix );
	glLoadMatrixf((GLfloat*)&mv_matrix);
}

_Bool FSSetView( RENDERMATRIX *matrix )
{
	memmove(&view_matrix,&matrix->m,sizeof(view_matrix));//memcpy
	reset_modelview();
	return true;
}

_Bool FSSetWorld( RENDERMATRIX *matrix )
{	
	memmove(&world_matrix,&matrix->m,sizeof(world_matrix));//memcpy
	reset_modelview();
	return true;
}

_Bool FSGetWorld(RENDERMATRIX *matrix)
{
	memmove(&matrix->m,&world_matrix,sizeof(matrix->m));//memcpy
	return true;
}

//
// using the concept of index/vertex buffers in opengl is a bit different
// so for now I'm just going going to return a pointer to memory
// then just convert the index/vertex buffer to a pure vertex buffer on draw
//
// the vertex/index pointers could probably end up being the glGen* id
// then internal display lists can be generated for the non dynamic functions
// so we can render the static objects as display lists
//

_Bool FSCreateVertexBuffer(RENDEROBJECT *renderObject, int numVertices)
{
	renderObject->lpVertexBuffer = malloc( numVertices * sizeof(LVERTEX) );
	return true;
}
_Bool FSCreateDynamicVertexBuffer(RENDEROBJECT *renderObject, int numVertices)
{FSCreateVertexBuffer(renderObject, numVertices); return true;}

_Bool FSCreateNormalBuffer(RENDEROBJECT *renderObject, int numNormals)
{ renderObject->lpNormalBuffer = malloc( numNormals * sizeof(NORMAL) ); return true; }

_Bool FSCreateDynamicNormalBuffer(RENDEROBJECT *renderObject, int numNormals)
{FSCreateNormalBuffer(renderObject, numNormals); return true;}

_Bool FSCreateIndexBuffer(RENDEROBJECT *renderObject, int numIndices)
{
	renderObject->lpIndexBuffer = malloc( numIndices * 3 * sizeof(WORD) );
	return true;
}
_Bool FSCreateDynamicIndexBuffer(RENDEROBJECT *renderObject, int numIndices)
{return FSCreateIndexBuffer(renderObject,numIndices);}

_Bool FSLockIndexBuffer(RENDEROBJECT *renderObject, WORD **indices)
{(*indices) = renderObject->lpIndexBuffer; return true;}

_Bool FSLockVertexBuffer(RENDEROBJECT *renderObject, LVERTEX **verts)
{(*verts) = renderObject->lpVertexBuffer; return true;}
_Bool FSUnlockIndexBuffer(RENDEROBJECT *renderObject){return true;}
_Bool FSUnlockVertexBuffer(RENDEROBJECT *renderObject){return true;}

_Bool FSLockNormalBuffer(RENDEROBJECT *renderObject, NORMAL **normals)
{(*normals) = renderObject->lpNormalBuffer; return true;}
_Bool FSUnlockNormalBuffer(RENDEROBJECT *renderObject){return true;}

_Bool FSCreateDynamic2dVertexBuffer(RENDEROBJECT *renderObject, int numVertices)
{
	renderObject->lpVertexBuffer = malloc( numVertices * sizeof(TLVERTEX) ); 
	return true;
}
_Bool FSLockPretransformedVertexBuffer(RENDEROBJECT *renderObject, TLVERTEX **verts)
{*verts = (void*)renderObject->lpVertexBuffer; return true;}

static void set_color( COLOR c )
{
	// COLOR is the value loaded from the files
	// it's packed as uchar[4] (bgra) and glColor expects (rgba)
	// so we flip the red/blue values with each other
	c = (c & 0xff00ff00) | ((c & 0x00ff0000) >> 16) | ((c & 0x000000ff) << 16);
	glColor4ubv((GLubyte*)&c);
}

int render_color_blend_red   = 0;
int render_color_blend_green = 0;
int render_color_blend_blue  = 0;

int render_lighting_enabled = 0;
int render_lighting_point_lights_only = 1;
int render_lighting_use_only_light_color = 0;
int render_lighting_use_only_light_color_and_blend = 0;

int render_light_ambience = 0;
int render_light_ambience_alpha = 255.0f;

int render_lighting_env_water         = 0;
int render_lighting_env_water_level   = 0;
float render_lighting_env_water_red   = 0.0f;
float render_lighting_env_water_green = 0.0f;
float render_lighting_env_water_blue  = 0.0f;

int render_lighting_env_whiteout = 0;

void render_reset_lighting_variables( void )
{
	render_color_blend_red   = 0;
	render_color_blend_green = 0;
	render_color_blend_blue  = 0;
	render_lighting_enabled = 0;
	render_lighting_point_lights_only = 1;
	render_lighting_use_only_light_color = 0;
	render_lighting_use_only_light_color_and_blend = 0;
	render_light_ambience = 0;
	render_light_ambience_alpha = 255.0f;
	render_lighting_env_water         = 0;
	render_lighting_env_water_level   = 0;
	render_lighting_env_water_red   = 0.0f;
	render_lighting_env_water_green = 0.0f;
	render_lighting_env_water_blue  = 0.0f;
	render_lighting_env_whiteout = 0;
}

void do_water_effect( VECTOR * pos, COLOR * color )
{
	u_int32_t r,g,b;
	int x,y,z;
	float intensity, seconds;
	static float speed = 71.0f;
	if( render_lighting_env_water == 2 && pos->y >= render_lighting_env_water_level )
		return;
	r = color[2] >> 2;
	g = color[1] >> 2;
	b = color[0] >> 2;
	x = (float)((int)(pos->x * 0.35f) % 360);
	y = (float)((int)(pos->y * 0.35f) % 360);
	z = (float)((int)(pos->z * 0.35f) % 360);
	seconds = SDL_GetTicks() / 1000.0f;
	intensity = (float) (
		( 
			sin( D2R( x + seconds * speed ) ) +  // cral = seconds * speed
			sin( D2R( y + seconds * speed ) ) + 
			sin( D2R( z + seconds * speed ) ) 
		) * 127.0F * 0.3333333F + 128.0F 
	);
	r += render_lighting_env_water_red   * intensity;
	g += render_lighting_env_water_green * intensity;
	b += render_lighting_env_water_blue  * intensity;
	if(r > 255) r = 255;
	if(g > 255) g = 255;
	if(b > 255) b = 255;
	color[2] = (u_int8_t) r;
	color[1] = (u_int8_t) g;
	color[0] = (u_int8_t) b;
}

void do_whiteout_effect( VECTOR * pos, COLOR * color )
{
	int x,y,z,intensity;
	float seconds;
	static float speed = 71.0f;
	x = (float)((int)(pos->x * 0.35f) % 360);
	y = (float)((int)(pos->y * 0.35f) % 360);
	z = (float)((int)(pos->z * 0.35f) % 360);
	seconds = SDL_GetTicks() / 1000.0f;
	intensity = (int) (
		( 
			sin( D2R( x + seconds * speed ) ) +  // cral = seconds * speed
			sin( D2R( y + seconds * speed ) ) + 
			sin( D2R( z + seconds * speed ) ) 
		) * 127.0F * 0.3333333F + 128.0F 
	);
	intensity += render_lighting_env_whiteout;
	if(intensity > 255) intensity = 255;
	*color &= 0xffff;
	*color |= ( intensity << 24 ) + ( intensity << 16 );
}

extern XLIGHT * FirstLightVisible;
void GetRealLightAmbientWorldSpace( VECTOR * Pos , float * R , float * G , float * B, float * A )
{
	VECTOR ray;
	float rlen, rlen2, lsize2, intensity, cosa, cosarc2;
	XLIGHT * LightPnt = FirstLightVisible;

	*R = *G = *B = render_light_ambience;
	*A = render_light_ambience_alpha;
	
	while( LightPnt )
	{
		ray.x = Pos->x - LightPnt->Pos.x;
		ray.y = Pos->y - LightPnt->Pos.y;
		ray.z = Pos->z - LightPnt->Pos.z;

		rlen2 = (
			ray.x * ray.x +
			ray.y * ray.y +
			ray.z * ray.z
		);

		lsize2 = LightPnt->Size * LightPnt->Size;
		
		if( rlen2 < lsize2 )
		{
			if(render_lighting_point_lights_only || LightPnt->Type == POINT_LIGHT)
			{
				intensity = 1.0F - rlen2 / (int) lsize2; 
			}
			else if(LightPnt->Type == SPOT_LIGHT)
			{
				if( rlen2 > 0.0F )
				{
					rlen = (float) sqrt( rlen2 );
					ray.x /= rlen;
					ray.y /= rlen;
					ray.z /= rlen;
				}
				
				cosa = ( 
					ray.x * LightPnt->Dir.x +
					ray.y * LightPnt->Dir.y + 
					ray.z * LightPnt->Dir.z
				);
				
				if ( rlen2 > lsize2 * 0.5F )
				{
					if ( cosa > LightPnt->CosArc )
						intensity = (
							( ( lsize2 - rlen2 ) / ( 0.75F * lsize2 ) ) *
							( ( cosa - LightPnt->CosArc ) / ( 1.0F - LightPnt->CosArc )	));
					else
						goto NEXT_LIGHT;
				}
				else if ( rlen2 > MIN_LIGHT_SIZE ) 
				{
					cosarc2 = LightPnt->CosArc * (
							1.0F - 
							( lsize2 * 0.5F - rlen2 ) / 
							( lsize2 * 0.5F - MIN_LIGHT_SIZE ) 
					);
					if ( cosa > cosarc2 )
						intensity = (
							( ( lsize2 - rlen2 ) / ( lsize2 - MIN_LIGHT_SIZE ) ) *
							( ( cosa - cosarc2 ) / ( 1.0F - cosarc2 ) ));
					else
						goto NEXT_LIGHT;
				}
				else 
				{
					intensity = ( cosa > 0.0F ) ? 1.0F : 1.0F + cosa ;
				}
			}
			else
			{
				DebugPrintf("Unknown light type %d\n",
					LightPnt->Type);
				goto NEXT_LIGHT;
			}

			*R += LightPnt->r * intensity;
			*G += LightPnt->g * intensity;
			*B += LightPnt->b * intensity;
			*A += 255.0f * intensity;
		}
		
NEXT_LIGHT:

		LightPnt = LightPnt->NextVisible;
	}

	if( *R > 255.0F ) *R = 255.0F;
	if( *G > 255.0F ) *G = 255.0F;
	if( *B > 255.0F ) *B = 255.0F;
	if( *A > 255.0F ) *A = 255.0F;
}

#define MINUS( X, Y )\
	tmp = X;\
	tmp -= Y;\
	X = (tmp < 0) ? 0 :\
		(tmp > 255) ? 255 : tmp

#define ADD( X, Y )\
	MINUS( X, -Y )

// color = (vert + light) - blend
// where blend = 255 - color
#define MIX_COLOR_BLEND_LIGHT( COLOR, BLEND, LIGHT )\
	ADD( COLOR, LIGHT );\
	MINUS( COLOR, BLEND )

void light_vert( LVERTEX * vert, COLOR * _color ) 
{
	int tmp;
	// work on color components individually
	u_int8_t *color = (u_int8_t *) _color;
	float r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;
	VECTOR world, v = {vert->x,vert->y,vert->z};
	MxV( &world_matrix, &v, &world );
	if( render_lighting_env_whiteout )
		do_whiteout_effect( &world, color );
	else if( render_lighting_env_water )
		do_water_effect( &world, color );
#ifndef LIGHT_EVERYTHING
	if( render_lighting_enabled )
#endif
		GetRealLightAmbientWorldSpace( &world, &r, &g, &b, &a );
	if(render_lighting_use_only_light_color)
	{
		color[0] = b;
		color[1] = g;
		color[2] = r;
		color[3] = a;
	}
	else if(render_lighting_use_only_light_color_and_blend)
	{
		color[0] = b;
		color[1] = g;
		color[2] = r;
		color[3] = a;
		ADD( color[0], render_color_blend_blue );
		ADD( color[1], render_color_blend_green );
		ADD( color[2], render_color_blend_red );
	}
	else
	{
		MIX_COLOR_BLEND_LIGHT( color[0], render_color_blend_blue,  b );
		MIX_COLOR_BLEND_LIGHT( color[1], render_color_blend_green, g );
		MIX_COLOR_BLEND_LIGHT( color[2], render_color_blend_red,   r );
	}
}

static void draw_vert( void * _vert, _Bool orthographic )
{
	LVERTEX * vert = (LVERTEX*) _vert;
	TLVERTEX * tlvert = (TLVERTEX*) _vert;
	if(orthographic)
	{
		set_color( tlvert->color );
		glTexCoord2f( tlvert->tu, tlvert->tv );
		glVertex2f( tlvert->x, tlvert->y );
	}
	else
	{
#ifdef NEW_LIGHTING
		COLOR c = vert->color;
		light_vert( vert, &c );
		set_color( c );
#else
		set_color( vert->color );
#endif
		glTexCoord2f( vert->tu, vert->tv );
		glVertex3f( vert->x, vert->y, vert->z );
	}
}

static _Bool draw_render_object( RENDEROBJECT *renderObject, int primitive_type, _Bool orthographic )
{
	int group;
	LVERTEX * verts = (LVERTEX*) renderObject->lpVertexBuffer;
	TLVERTEX * tlverts = (TLVERTEX*) renderObject->lpVertexBuffer;
	WORD * indices = (WORD*) renderObject->lpIndexBuffer;

	//assert(renderObject->vbLocked == 0);
	
	if(orthographic)
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0.0, (double)render_info.ThisMode.w, 0.0, (double)render_info.ThisMode.h);
		// These next two steps allow us to specify screen location from top/left offets
		// invert the y axis, down is positive
		glScalef(1, -1, 1);
		// and move the origin from the bottom left corner to the upper left corner
		glTranslatef(0.0f, -((float)render_info.ThisMode.h), 0.0f);
	}

	for (group = 0; group < renderObject->numTextureGroups; group++)
	{
		int i;
		int startVert  = renderObject->textureGroups[group].startVert;
		int numVerts   = renderObject->textureGroups[group].numVerts;

		if(renderObject->textureGroups[group].colourkey)
			set_alpha_ignore();

		if( renderObject->textureGroups[group].texture )
		{
			GLuint texture = *(GLuint*)renderObject->textureGroups[group].texture;
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, texture);
		}

		glBegin(primitive_type);

		// draw vertex list using index list
		if(renderObject->lpIndexBuffer)
		{
			int startIndex = renderObject->textureGroups[group].startIndex;
			int numIndices = renderObject->textureGroups[group].numTriangles * 3;
			for( i = 0; i < numIndices; i++ )
			{
				int indice = indices[ startIndex + i ];
				int vert = startVert + indice;
				if(orthographic)
					draw_vert( &tlverts[vert], orthographic );
				else
					draw_vert( &verts[vert], orthographic );
			}
		}
		// draw using only vertex list
		else
		{
			for( i = startVert; i < numVerts; i++ )
				if(orthographic)
					draw_vert( &tlverts[i], orthographic );
				else
					draw_vert( &verts[i], orthographic );
		}
		
		glEnd();

		if( renderObject->textureGroups[group].texture )
			glDisable(GL_TEXTURE_2D);

		if(renderObject->textureGroups[group].colourkey)
			unset_alpha_ignore();
	}

	if(orthographic)
	{
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
	}

	return true;
}

_Bool draw_object(RENDEROBJECT *renderObject){return draw_render_object(renderObject,GL_TRIANGLES,false);}
_Bool draw_2d_object(RENDEROBJECT *renderObject){return draw_render_object(renderObject,GL_TRIANGLES,true);}
_Bool draw_line_object(RENDEROBJECT *renderObject){return draw_render_object(renderObject,GL_LINES,false);}

void FSReleaseRenderObject(RENDEROBJECT *renderObject)
{
	int i;
	if (renderObject->lpVertexBuffer)
	{
		free(renderObject->lpVertexBuffer);
		renderObject->lpVertexBuffer = NULL;
	}
	if (renderObject->lpIndexBuffer)
	{
		free(renderObject->lpIndexBuffer);
		renderObject->lpIndexBuffer = NULL;
	}
	for (i = 0; i < renderObject->numTextureGroups; i++)
	{
		renderObject->textureGroups[i].numVerts = 0;
		renderObject->textureGroups[i].startVert = 0;

		if (renderObject->textureGroups[i].texture)
		{
			// tload.c calls release_texture
			// we should not release them here
			renderObject->textureGroups[i].texture = NULL;
		}
	}
	renderObject->numTextureGroups = 0;
}

#endif // OPENGL
