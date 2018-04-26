/*
 * this is auto generated code, please do not modify
 * generated by codegen.exe
 */

#pragma once

#include "insigne/gl/identifiers.h"
#include "oglapis.h"

namespace insigne {

void pxActiveTexture(GLenum texture);

void pxAttachShader(GLuint program, GLuint shader);

void pxBindBuffer(GLenum target, GLuint buffer);

void pxBindFramebuffer(GLenum target, GLuint framebuffer);

void pxBindRenderbuffer(GLenum target, GLuint renderbuffer);

void pxBindTexture(GLenum target, GLuint texture);

void pxBindVertexArray(GLuint targetArray);

void pxBlendEquation(GLenum mode);

void pxBlendFunc(GLenum sfactor, GLenum dfactor);

void pxBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);

void pxBufferData(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);

GLenum pxCheckFramebufferStatus(GLenum target);

void pxClear(GLbitfield mask);

void pxClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);

void pxCompileShader(GLuint shader);

GLuint pxCreateProgram();

GLuint pxCreateShader(GLenum shaderType);

void pxCullFace(GLenum mode);

void pxDeleteBuffers(GLsizei n, const GLuint* buffers);

void pxDeleteShader(GLuint shader);

void pxDeleteVertexArrays(GLsizei n, const GLuint* arrays);

void pxDepthFunc(GLenum func);

void pxDepthMask(GLboolean flag);

void pxDisable(GLenum cap);

void pxDrawBuffers(GLsizei n, const GLenum* bufs);

void pxDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices);

void pxEnable(GLenum cap);

void pxEnableVertexAttribArray(GLuint index);

void pxFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);

void pxFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);

void pxFrontFace(GLenum mode);

void pxGenBuffers(GLsizei n, GLuint* buffers);

void pxGenFramebuffers(GLsizei n, GLuint* framebuffers);

void pxGenRenderbuffers(GLsizei n, GLuint* renderbuffers);

void pxGenTextures(GLsizei n, GLuint* textures);

void pxGenVertexArrays(GLsizei n, GLuint* arrays);

void pxGenerateMipmap(GLenum target);

GLint pxGetAttribLocation(GLuint program, const GLchar* name);

GLenum pxGetError();

void pxGetIntegerv(GLenum pname, GLint* data);

void pxGetShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei* length, GLchar* infoLog);

void pxGetShaderiv(GLuint shader, GLenum pname, GLint* params);

GLint pxGetUniformLocation(GLuint program, const GLchar* name);

void pxLinkProgram(GLuint program);

void pxRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);

void pxRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalFormat, GLsizei width, GLsizei height);

void pxScissor(GLint x, GLint y, GLsizei width, GLsizei height);

void pxShaderSource(GLuint shader, GLsizei count, const GLchar** string, const GLint* length);

void pxTexImage2D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* data);

void pxTexParameteri(GLenum target, GLenum pname, GLint param);

void pxUniform1f(GLint location, GLfloat v0);

void pxUniform1i(GLint location, GLint v0);

void pxUniform3fv(GLint location, GLsizei count, const GLfloat* value);

void pxUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);

void pxUseProgram(GLuint program);

void pxVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);

void pxViewport(GLint x, GLint y, GLsizei width, GLsizei height);

const GLubyte* pxGetString(GLenum name);

}
