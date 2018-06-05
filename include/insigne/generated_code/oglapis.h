
/*
 * this is auto generated code, please do not modify
 * generated by codegen.exe
 */

#pragma once

#include "insigne/gl/identifiers.h"

#if defined(PLATFORM_WINDOWS)
#	ifndef APIENTRY
#		define APIENTRY __stdcall
#	endif
#	ifndef APICALL
#		define APICALL __declspec(dllimport)
#	endif
#else
#	ifndef APIENTRY
#		define APIENTRY
#	endif
#	ifndef APICALL
#		define APICALL
#	endif
#endif

namespace insigne {
typedef APICALL void (APIENTRY *glActiveTexture_t)(GLenum texture);
extern glActiveTexture_t glActiveTexture;

typedef APICALL void (APIENTRY *glAttachShader_t)(GLuint program, GLuint shader);
extern glAttachShader_t glAttachShader;

typedef APICALL void (APIENTRY *glBindBuffer_t)(GLenum target, GLuint buffer);
extern glBindBuffer_t glBindBuffer;

typedef APICALL void (APIENTRY *glBindFramebuffer_t)(GLenum target, GLuint framebuffer);
extern glBindFramebuffer_t glBindFramebuffer;

typedef APICALL void (APIENTRY *glBindRenderbuffer_t)(GLenum target, GLuint renderbuffer);
extern glBindRenderbuffer_t glBindRenderbuffer;

typedef APICALL void (APIENTRY *glBindTexture_t)(GLenum target, GLuint texture);
extern glBindTexture_t glBindTexture;

typedef APICALL void (APIENTRY *glBindVertexArray_t)(GLuint targetArray);
extern glBindVertexArray_t glBindVertexArray;

typedef APICALL void (APIENTRY *glBlendEquation_t)(GLenum mode);
extern glBlendEquation_t glBlendEquation;

typedef APICALL void (APIENTRY *glBlendFunc_t)(GLenum sfactor, GLenum dfactor);
extern glBlendFunc_t glBlendFunc;

typedef APICALL void (APIENTRY *glBlitFramebuffer_t)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
extern glBlitFramebuffer_t glBlitFramebuffer;

typedef APICALL void (APIENTRY *glBufferData_t)(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
extern glBufferData_t glBufferData;

typedef APICALL GLenum (APIENTRY *glCheckFramebufferStatus_t)(GLenum target);
extern glCheckFramebufferStatus_t glCheckFramebufferStatus;

typedef APICALL void (APIENTRY *glClear_t)(GLbitfield mask);
extern glClear_t glClear;

typedef APICALL void (APIENTRY *glClearColor_t)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern glClearColor_t glClearColor;

typedef APICALL void (APIENTRY *glCompileShader_t)(GLuint shader);
extern glCompileShader_t glCompileShader;

typedef APICALL GLuint (APIENTRY *glCreateProgram_t)();
extern glCreateProgram_t glCreateProgram;

typedef APICALL GLuint (APIENTRY *glCreateShader_t)(GLenum shaderType);
extern glCreateShader_t glCreateShader;

typedef APICALL void (APIENTRY *glCullFace_t)(GLenum mode);
extern glCullFace_t glCullFace;

typedef APICALL void (APIENTRY *glDeleteBuffers_t)(GLsizei n, const GLuint* buffers);
extern glDeleteBuffers_t glDeleteBuffers;

typedef APICALL void (APIENTRY *glDeleteShader_t)(GLuint shader);
extern glDeleteShader_t glDeleteShader;

typedef APICALL void (APIENTRY *glDeleteVertexArrays_t)(GLsizei n, const GLuint* arrays);
extern glDeleteVertexArrays_t glDeleteVertexArrays;

typedef APICALL void (APIENTRY *glDepthFunc_t)(GLenum func);
extern glDepthFunc_t glDepthFunc;

typedef APICALL void (APIENTRY *glDepthMask_t)(GLboolean flag);
extern glDepthMask_t glDepthMask;

typedef APICALL void (APIENTRY *glDisable_t)(GLenum cap);
extern glDisable_t glDisable;

typedef APICALL void (APIENTRY *glDrawBuffers_t)(GLsizei n, const GLenum* bufs);
extern glDrawBuffers_t glDrawBuffers;

typedef APICALL void (APIENTRY *glDrawElements_t)(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices);
extern glDrawElements_t glDrawElements;

typedef APICALL void (APIENTRY *glEnable_t)(GLenum cap);
extern glEnable_t glEnable;

typedef APICALL void (APIENTRY *glEnableVertexAttribArray_t)(GLuint index);
extern glEnableVertexAttribArray_t glEnableVertexAttribArray;

typedef APICALL void (APIENTRY *glFramebufferRenderbuffer_t)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
extern glFramebufferRenderbuffer_t glFramebufferRenderbuffer;

typedef APICALL void (APIENTRY *glFramebufferTexture2D_t)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
extern glFramebufferTexture2D_t glFramebufferTexture2D;

typedef APICALL void (APIENTRY *glFrontFace_t)(GLenum mode);
extern glFrontFace_t glFrontFace;

typedef APICALL void (APIENTRY *glGenBuffers_t)(GLsizei n, GLuint* buffers);
extern glGenBuffers_t glGenBuffers;

typedef APICALL void (APIENTRY *glGenFramebuffers_t)(GLsizei n, GLuint* framebuffers);
extern glGenFramebuffers_t glGenFramebuffers;

typedef APICALL void (APIENTRY *glGenRenderbuffers_t)(GLsizei n, GLuint* renderbuffers);
extern glGenRenderbuffers_t glGenRenderbuffers;

typedef APICALL void (APIENTRY *glGenTextures_t)(GLsizei n, GLuint* textures);
extern glGenTextures_t glGenTextures;

typedef APICALL void (APIENTRY *glGenVertexArrays_t)(GLsizei n, GLuint* arrays);
extern glGenVertexArrays_t glGenVertexArrays;

typedef APICALL void (APIENTRY *glGenerateMipmap_t)(GLenum target);
extern glGenerateMipmap_t glGenerateMipmap;

typedef APICALL GLint (APIENTRY *glGetAttribLocation_t)(GLuint program, const GLchar* name);
extern glGetAttribLocation_t glGetAttribLocation;

typedef APICALL GLenum (APIENTRY *glGetError_t)();
extern glGetError_t glGetError;

typedef APICALL void (APIENTRY *glGetIntegerv_t)(GLenum pname, GLint* data);
extern glGetIntegerv_t glGetIntegerv;

typedef APICALL void (APIENTRY *glGetShaderInfoLog_t)(GLuint shader, GLsizei maxLength, GLsizei* length, GLchar* infoLog);
extern glGetShaderInfoLog_t glGetShaderInfoLog;

typedef APICALL void (APIENTRY *glGetShaderiv_t)(GLuint shader, GLenum pname, GLint* params);
extern glGetShaderiv_t glGetShaderiv;

typedef APICALL const GLubyte* (APIENTRY *glGetString_t)(GLenum name);
extern glGetString_t glGetString;

typedef APICALL GLint (APIENTRY *glGetUniformLocation_t)(GLuint program, const GLchar* name);
extern glGetUniformLocation_t glGetUniformLocation;

typedef APICALL void (APIENTRY *glLinkProgram_t)(GLuint program);
extern glLinkProgram_t glLinkProgram;

typedef APICALL void (APIENTRY *glPixelStorei_t)(GLenum pname, GLint param);
extern glPixelStorei_t glPixelStorei;

typedef APICALL void (APIENTRY *glRenderbufferStorage_t)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
extern glRenderbufferStorage_t glRenderbufferStorage;

typedef APICALL void (APIENTRY *glRenderbufferStorageMultisample_t)(GLenum target, GLsizei samples, GLenum internalFormat, GLsizei width, GLsizei height);
extern glRenderbufferStorageMultisample_t glRenderbufferStorageMultisample;

typedef APICALL void (APIENTRY *glScissor_t)(GLint x, GLint y, GLsizei width, GLsizei height);
extern glScissor_t glScissor;

typedef APICALL void (APIENTRY *glShaderSource_t)(GLuint shader, GLsizei count, const GLchar** string, const GLint* length);
extern glShaderSource_t glShaderSource;

typedef APICALL void (APIENTRY *glStencilFunc_t)(GLenum func, GLint ref, GLuint mask);
extern glStencilFunc_t glStencilFunc;

typedef APICALL void (APIENTRY *glStencilOp_t)(GLenum sfail, GLenum dpfail, GLenum dppass);
extern glStencilOp_t glStencilOp;

typedef APICALL void (APIENTRY *glTexImage2D_t)(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* data);
extern glTexImage2D_t glTexImage2D;

typedef APICALL void (APIENTRY *glTexParameteri_t)(GLenum target, GLenum pname, GLint param);
extern glTexParameteri_t glTexParameteri;

typedef APICALL void (APIENTRY *glUniform1f_t)(GLint location, GLfloat v0);
extern glUniform1f_t glUniform1f;

typedef APICALL void (APIENTRY *glUniform1i_t)(GLint location, GLint v0);
extern glUniform1i_t glUniform1i;

typedef APICALL void (APIENTRY *glUniform3fv_t)(GLint location, GLsizei count, const GLfloat* value);
extern glUniform3fv_t glUniform3fv;

typedef APICALL void (APIENTRY *glUniformMatrix4fv_t)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
extern glUniformMatrix4fv_t glUniformMatrix4fv;

typedef APICALL void (APIENTRY *glUseProgram_t)(GLuint program);
extern glUseProgram_t glUseProgram;

typedef APICALL void (APIENTRY *glVertexAttribPointer_t)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);
extern glVertexAttribPointer_t glVertexAttribPointer;

typedef APICALL void (APIENTRY *glViewport_t)(GLint x, GLint y, GLsizei width, GLsizei height);
extern glViewport_t glViewport;

}
