#define GL_GLEXT_PROTOTYPES

#include <GL/gl.h>
#include <gtk/gtk.h>
#include <vlc/vlc.h>
#include "libvlc_gtkglarea.h"

const char *VERTEX_SOURCE =
    "#version 330\n"
    ""
    "in vec2 vertex;\n"
    "out vec2 uv;\n"
    ""
    "void main() {\n"
    "uv = vertex;\n"
    "   gl_Position = vec4(2.0*vertex.x - 1.0, 1.0 - 2.0*vertex.y, 0.0, 1.0);\n"
    "}";

// The RV24 format exchanges the red and blue channels, so we must exchange them
// before displaying the color
// see: https://code.videolan.org/videolan/vlc/-/issues/20475
const char *FRAGMENT_SOURCE =
    "#version 330\n"
    ""
    "in vec2 uv;\n"
    "out vec3 outputColor;\n"
    "uniform sampler2D image; \n"
    ""
    "void main() {\n"
    "   vec3 color = texture(image, uv).rgb;\n"
    "   outputColor = vec3(color.z, color.y ,color.x);\n"
    "}";

GLfloat VERTICES[] = {
    -1, -1, 0, // left  bottom
    -1, 1, 0,  // left  top
    1, 1, 0,   // right top
    1, -1, 0}; // right bottom

GLubyte INDICES[] = {
    0, 1, 2,  // triangle 1: left bottom / left  top    / right top
    0, 3, 2}; // triangle 2: left bottom / right bottom / right top

struct Data
{
  GMutex *mutex;
  gint area_width;
  gint area_height;
  unsigned int video_width;
  unsigned int video_height;
  unsigned int render_count;
  unsigned char *pixel_buffer;
  GLuint textureId;
  GLuint position_buffer;
  GLuint program;
  GLuint vao;
};

struct Data *data_new()
{
  struct Data *data = malloc(sizeof(struct Data));

  data->mutex = malloc(sizeof(GMutex));
  data->pixel_buffer = NULL;
  data->area_width = 0;
  data->area_height = 0;
  data->video_width = 0;
  data->video_height = 0;
  data->render_count = 0;
  data->textureId = 0;

  g_mutex_init(data->mutex);

  return data;
}

void data_free(struct Data *data)
{
  free(data->mutex);
  if (data->pixel_buffer != NULL)
  {
    free(data->pixel_buffer);
  }
}

unsigned
setup_video(
    void **opaque,
    char *chroma,
    unsigned int *width,
    unsigned int *height,
    unsigned int *pitches,
    unsigned int *lines)
{
  struct Data *data = (struct Data *)*opaque;

  g_mutex_lock(data->mutex);

  // save size & setup buffer
  data->video_width = *width;
  data->video_height = *height;
  data->pixel_buffer =
      (unsigned char *)malloc(
          data->video_width * data->video_height * 3 * sizeof(unsigned char));

  // setup vlc
  memcpy(chroma, "RV24", 4);
  (*pitches) = data->video_width * 3;
  (*lines) = data->video_height;

  g_mutex_unlock(data->mutex);

  return 1; // only one color plane
}

void cleanup_video(void *opaque)
{
  struct Data *data = (struct Data *)opaque;

  g_free(data->pixel_buffer);
}

void init_buffers(GLuint *vao, GLuint *buffer)
{
  // vao
  glGenVertexArrays(1, vao);
  glBindVertexArray(*vao);

  // store vertices
  glGenBuffers(1, buffer);
  glBindBuffer(GL_ARRAY_BUFFER, *buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GLuint create_shader(int type, const char *source)
{
  GLuint shader = glCreateShader(type);
  if (type == GL_FRAGMENT_SHADER)
  {
    glShaderSource(shader, 1, &source, NULL);
  }
  if (type == GL_VERTEX_SHADER)
  {
    glShaderSource(shader, 1, &source, NULL);
  }
  glCompileShader(shader);

  int status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE)
  {
    int log_len;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);

    char *buffer = g_malloc(log_len + 1);
    glGetShaderInfoLog(shader, log_len, NULL, buffer);

    g_warning(
        "%s shader error: %s",
        type == GL_VERTEX_SHADER ? "vertex" : "fragment",
        buffer);

    g_free(buffer);
    glDeleteShader(shader);

    return 0;
  }

  return shader;
}

void init_shaders(GLuint *program_out)
{
  GLuint vertex = create_shader(GL_VERTEX_SHADER, VERTEX_SOURCE);
  if (vertex == 0)
  {
    *program_out = 0;
    return;
  }

  GLuint fragment = create_shader(GL_FRAGMENT_SHADER, FRAGMENT_SOURCE);
  if (fragment == 0)
  {
    glDeleteShader(vertex);
    *program_out = 0;
    return;
  }

  GLuint program = glCreateProgram();
  glAttachShader(program, vertex);
  glAttachShader(program, fragment);
  glLinkProgram(program);

  int status;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (status == GL_FALSE)
  {
    int log_len;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len);

    char *buffer = g_malloc(log_len + 1);
    glGetProgramInfoLog(program, log_len, NULL, buffer);

    g_warning("program linking failure: %s", buffer);
    g_free(buffer);

    glDeleteProgram(program);
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    *program_out = 0;
    return;
  }

  glDetachShader(program, vertex);
  glDetachShader(program, fragment);
  glDeleteShader(vertex);
  glDeleteShader(fragment);

  *program_out = program;
}

void init_texture(GLuint *textureId)
{
  glGenTextures(1, textureId);
  glBindTexture(GL_TEXTURE_2D, *textureId);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
}

void *video_lock_cb(void *opaque, void **planes)
{
  struct Data *data = (struct Data *)opaque;

  g_mutex_lock(data->mutex);
  planes[0] = data->pixel_buffer;
  return NULL;
}

void video_unlock_cb(void *opaque, void *picture, void *const *planes)
{
  struct Data *data = (struct Data *)opaque;

  g_mutex_unlock(data->mutex);
}

void video_display_cb(void *opaque, void *picture) {}

void realize(GtkGLArea *area, gpointer user_data)
{
  struct Data *data = (struct Data *)user_data;

  // setup opengl
  gtk_gl_area_make_current(area);
  init_buffers(&data->vao, &data->position_buffer);
  init_shaders(&data->program);
  init_texture(&data->textureId);

  // setup area update timer
  GdkFrameClock *frame_clock = gtk_widget_get_frame_clock(GTK_WIDGET(area));
  g_signal_connect_swapped(
      frame_clock, "update", G_CALLBACK(gtk_gl_area_queue_render), area);
  gdk_frame_clock_begin_updating(frame_clock);
}

void resize(
    GtkGLArea *area,
    gint width,
    gint height,
    gpointer user_data)
{
  struct Data *data = (struct Data *)user_data;

  data->area_width = width;
  data->area_height = height;
}

void unrealize(GtkGLArea *area, gpointer user_data)
{
  struct Data *data = (struct Data *)user_data;

  gtk_gl_area_make_current(area);
  glDeleteBuffers(1, &data->position_buffer);
  glDeleteProgram(data->program);
  data_free(data);
}

void set_texture(struct Data *data)
{
  g_mutex_lock(data->mutex);
  glBindTexture(GL_TEXTURE_2D, data->textureId);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGB,
      data->video_width,
      data->video_height,
      0,
      GL_BGR_EXT,
      GL_UNSIGNED_BYTE,
      data->pixel_buffer);
  g_mutex_unlock(data->mutex);
}

void draw_square(struct Data *data)
{
  // setup
  glUseProgram(data->program);
  glBindBuffer(GL_ARRAY_BUFFER, data->position_buffer);
  glEnableVertexAttribArray(0);

  // draw
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, INDICES);

  // cleanup
  glDisableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glUseProgram(0);
}

gboolean render(GtkGLArea *area, GdkGLContext *context, gpointer user_data)
{
  struct Data *data = (struct Data *)user_data;

  // clear viewport
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);

  // wait buffer initialization
  g_mutex_lock(data->mutex);
  bool buffer_not_initialized = data->pixel_buffer == NULL;
  g_mutex_unlock(data->mutex);
  if (buffer_not_initialized)
  {
    return TRUE;
  }

  // FIXME: prevent glTexImage2D segmentation fault
  if (data->render_count < 24)
  {
    data->render_count++;
    return TRUE;
  }

  // set aspect ratio
  float video_aspect = (float)data->video_width / (float)data->video_height;
  float area_aspect = (float)data->area_width / (float)data->area_height;
  if (video_aspect > area_aspect)
  {
    int height = data->area_width / video_aspect;
    glViewport(
        0, (data->area_height / 2) - height / 2, data->area_width, height);
  }
  else
  {
    int width = data->area_height * video_aspect;
    glViewport((data->area_width / 2) - width / 2, 0, width, data->area_height);
  }

  // draw
  set_texture(data);
  draw_square(data);

  // flush rendering
  glFlush();

  return TRUE;
}

void libvlc_media_player_set_gtkglarea(
    libvlc_media_player_t *media_player,
    GtkGLArea *area)
{
  // data
  struct Data *data = data_new();

  // setup area
  g_signal_connect(area, "resize", G_CALLBACK(resize), data);
  g_signal_connect(area, "realize", G_CALLBACK(realize), data);
  g_signal_connect(area, "render", G_CALLBACK(render), data);
  g_signal_connect(area, "unrealize", G_CALLBACK(unrealize), data);

  // setup libvlc
  libvlc_video_set_callbacks(
      media_player, video_lock_cb, video_unlock_cb, video_display_cb, data);
  libvlc_video_set_format_callbacks(media_player, setup_video, cleanup_video);
}
